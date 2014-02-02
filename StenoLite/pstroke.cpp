#include "stdafx.h"
#include "pstroke.h"

#include "globals.h"
#include "stenodata.h"
#include <string>
#include <list>
#include <stack>
#include <queue>
#include "texthelpers.h"
#include "setmode.h"
#include "newentrydlg.h"
#include "search.h"
#include "addendings.h"
#include "pview.h"
#include <Richedit.h>
#include "resource.h"

void InnerProcess(unsigned __int8* stroke, std::list<singlestroke*>::iterator &insert, std::list<singlestroke*> * target);
bool spaceafter(std::list<singlestroke*>::iterator it, std::list<singlestroke*>* target, bool erase);
bool spacebefore(std::list<singlestroke*>::iterator it, std::list<singlestroke*>* target, bool erase);

inline bool compare3(const unsigned __int8* a, const unsigned __int8* b) {
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

void addStroke(__int32 s) {
	WaitForSingleObject(sharedData.protectqueue, INFINITE);
	sharedData.inputs.push(s);
	ReleaseMutex(sharedData.protectqueue);
	SetEvent(sharedData.newentry);
}

void sendstroke(unsigned __int8* keys) {
	TCHAR buffer[32] = TEXT("\r\n");
	if (sharedData.currentd != NULL)
		stroketosteno(keys, &buffer[2], sharedData.currentd->format);
	else
		stroketosteno(keys, &buffer[2], TEXT("#STKPWHRAO*EUFRPBLGTSDZ"));

	int lines = SendMessage(controls.mestroke, EM_GETLINECOUNT, 0, 0);
	if (lines > controls.numlines - 1) {
		int end = SendMessage(controls.mestroke, EM_LINEINDEX, 1, 0);
		SendMessage(controls.mestroke, EM_SETSEL, 0, end);
		SendMessage(controls.mestroke, EM_REPLACESEL, FALSE, (LPARAM)TEXT(""));
	}

	int len = SendMessage(controls.mestroke, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(controls.mestroke, EM_SETSEL, len, len);
	SendMessage(controls.mestroke, EM_REPLACESEL, FALSE, (LPARAM)buffer);

	__int32* val = (__int32*)keys;
	addStroke(*val);

	keys[0] = keys[1] = keys[2] = keys[4] = 0;
}

void trimStokesList() {
	if (sharedData.strokes.size() > 100) {
		singlestroke* st = sharedData.strokes.back();
		textoutput* t = sharedData.strokes.back()->textout;
		sharedData.strokes.pop_back();
		if (t != sharedData.strokes.back()->textout) {
			delete t;
		}
		delete st;
	}
}

tstring sendText(tstring fulltext, unsigned __int8 &flags, unsigned __int8 prevflags, unsigned __int8 nextflags, BOOL &trimspace, bool redirect) {
	flags = 0;
	tstring::const_iterator i = fulltext.cbegin();
	HKL locale = GetKeyboardLayout(GetCurrentThreadId());

	bool insubcommand = false;
	bool inescape = false;

	tstring finaltext(TEXT(""));

	INPUT* inputs = new INPUT[fulltext.length() * 4 + 4];
	memset(inputs, 0, sizeof(INPUT)*(fulltext.length() * 4 + 4));
	int index = 2;
	std::stack<WORD> subseqstack;

	bool spaceinfront = false;
	bool first = true;
	bool poped = false;

	while (i != fulltext.cend() ) {
		if (inescape) {
			if (*i == TEXT('p')) {
				inescape = false;
				insubcommand = true;

				if (!inputstate.textstack.empty()) {
					unsigned __int8 newflags = 0;
					tstring cmd = inputstate.textstack.top();
					inputstate.textstack.pop();
					tstring result = sendText(cmd, newflags, prevflags, nextflags, trimspace, redirect);
					finaltext = result + finaltext;
					poped = true;
				}
			}
			else if (*i == TEXT('A')) {
				inescape = false;
				insubcommand = true;

				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = VK_MENU;

				subseqstack.push(VK_MENU);
				index++;

				if (i + 1 != fulltext.cend()) {
					if (*(i + 1) == TEXT('[')) {
						i++;
					}
				}
			}
			else if (*i == TEXT('C')) {
				inescape = false;
				insubcommand = true;

				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = VK_CONTROL;

				subseqstack.push(VK_CONTROL);
				index++;

				if (i + 1 != fulltext.cend()) {
					if (*(i + 1) == TEXT('[')) {
						i++;
					}
				}
			}
			else if (*i == TEXT('S')) {
				inescape = false;
				insubcommand = true;

				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = VK_SHIFT;

				subseqstack.push(VK_SHIFT);
				index++;

				if (i + 1 != fulltext.cend()) {
					if (*(i + 1) == TEXT('[')) {
						i++;
					}
				}
			}
			else if (*i == TEXT('-')) {
				flags |= TF_LOWNEXT;
				inescape = false;
			}
			else if (*i == TEXT('+')) {
				flags |= TF_CAPNEXT;
				inescape = false;
			}
			else if (*i == TEXT('?')) {
				if (projectdata.open)
					PostMessage(projectdata.dlg, WM_NEWITEMDLG, 0, 0);
				else
					PostMessage(controls.main, WM_NEWITEMDLG, 0, 0);
				inescape = false;
			}
			else if (*i == TEXT('t')) {
				SHORT rval = VK_TAB;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;

				finaltext += TEXT('\t');
			}
			else if (*i == TEXT('c')) {
				SHORT rval = VK_CAPITAL;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == TEXT('h')) {
				SHORT rval = VK_LEFT;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == TEXT('k')) {
				SHORT rval = VK_UP;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == TEXT('j')) {
				SHORT rval = VK_DOWN;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == TEXT('l')) {
				SHORT rval = VK_RIGHT;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == TEXT('n')) {
				SHORT rval = VK_RETURN;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
				if (projectdata.open)
					finaltext += TEXT("\r\n");
				else
					finaltext += TEXT('\n');
			}
			else if (*i == TEXT('b')) {
				SHORT rval = VK_BACK;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == TEXT('x')) {
				SHORT rval = VK_ESCAPE;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == TEXT('d')) {
				SHORT rval = VK_DELETE;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == TEXT('F')) {
				SHORT rval = VK_F1;
				switch (_wtoi(getSubSeq(i, fulltext.cend()).c_str())) {
				case 2: rval = VK_F2; break;
				case 3: rval = VK_F3; break;
				case 4: rval = VK_F4; break;
				case 5: rval = VK_F5; break;
				case 6: rval = VK_F6; break;
				case 7: rval = VK_F7; break;
				case 8: rval = VK_F8; break;
				case 9: rval = VK_F9; break;
				case 10: rval = VK_F10; break;
				case 11: rval = VK_F11; break;
				case 12: rval = VK_F12; break;
				}

				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == TEXT('M')) {
				int m = _wtoi(getSubSeq(i, fulltext.cend()).c_str());
				setMode(m);
				settings.mode = m;
				SendMessage(controls.inputs, CB_SETCURSEL, (WPARAM)m, (LPARAM)0);
				inescape = false;
			}
			else if (*i == TEXT('D')) {
				tstring dictname = getSubSeq(i, fulltext.cend());
				std::list<std::tuple<tstring, dictionary*>>::iterator di = sharedData.dicts.begin();
				while (di != sharedData.dicts.cend()) {
					if (std::get<0, tstring, dictionary*>(*di).compare(dictname) == 0) {
						settings.dict = dictname;
						setDictionary(std::get<1, tstring, dictionary*>(*di));		
						int sel = SendMessage(controls.dicts, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)(dictname.c_str()));
						SendMessage(controls.dicts, CB_SETCURSEL, (WPARAM)sel, (LPARAM)0);
					}
					di++;
				}
				inescape = false;
			}
			else if (*i == TEXT('P')) {
				tstring command = getSubSeq(i, fulltext.cend());
				inputstate.textstack.push(command);

				inescape = false;
			}
			else{
				//other, posibly \] in subcommand -> add key to outputs

				SHORT rval = VkKeyScanEx(*i, locale);
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
				if (!insubcommand) {
					finaltext += *i;
				}
			}
		}
		else if (insubcommand) {
			if (*i == TEXT(']')) {
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index].ki.wVk = subseqstack.top();

				index += 1;
				subseqstack.pop();
				if (subseqstack.empty()) {
					insubcommand = false;
				}
			}
			else if (*i == TEXT('\\')) {
				inescape = true;
			}
			else {
				SHORT rval = VkKeyScanEx(*i, locale);
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;

				//note, not added to text output
			}
		}
		else if (*i == TEXT('^')) {
			if (i == fulltext.cbegin()) {
				flags = flags | TF_INOSPACE;
			}
			else {
				flags = flags | TF_ENOSPACE;
			}
		}
		else if (*i == TEXT('&')) {
			if (i == fulltext.cbegin()) {
				flags = flags | TF_IPSPACE;
			}
			else {
				flags = flags | TF_EPSPACE;
			}
		}
		else if (*i == TEXT('\\')) {
			inescape = true;
		}
		else {
			
			SHORT rval = VkKeyScanEx(*i, locale);
			if (((HIBYTE(rval) & 1) != 0 || (first && (TF_CAPNEXT & prevflags) != 0)) && (!first || (TF_LOWNEXT & prevflags) == 0)){
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = VK_SHIFT;

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				inputs[index + 2].type = INPUT_KEYBOARD;
				inputs[index + 2].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 2].ki.wVk = LOBYTE(rval);

				inputs[index + 3].type = INPUT_KEYBOARD;
				inputs[index + 3].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 3].ki.wVk = VK_SHIFT;

				index += 4;

				finaltext += towupper(*i);
			}
			else {
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;

				finaltext += towlower(*i);
			}
			first = false;
		}
		i++;
	}


	while (!subseqstack.empty()) {
		inputs[index].type = INPUT_KEYBOARD;
		inputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
		inputs[index].ki.wVk = subseqstack.top();

		index += 1;
		subseqstack.pop();
	}

	if ((flags & TF_INOSPACE) == TF_INOSPACE || (prevflags & TF_ENOSPACE) == TF_ENOSPACE || ((flags & TF_IPSPACE) == TF_IPSPACE && (prevflags & TF_EPSPACE) == TF_EPSPACE)) {
		//no additionalspace
		if (settings.space == 1 && (prevflags & TF_ENOSPACE) == 0) {
			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_BACK;

			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			inputs[1].ki.wVk = VK_BACK;

			if (!poped) {
				trimspace = TRUE;
			}

			spaceinfront = true;
		}
	}
	else if (settings.space == 0 && !poped && finaltext.length() > 0) {

		inputs[0].type = INPUT_KEYBOARD;
		inputs[0].ki.wVk = VK_SPACE;

		inputs[1].type = INPUT_KEYBOARD;
		inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
		inputs[1].ki.wVk = VK_SPACE;

		finaltext = TEXT(' ') + finaltext;
		spaceinfront = true;
	}

	if ((flags & TF_ENOSPACE) != TF_ENOSPACE && settings.space == 1 && finaltext.length() > 0 && (nextflags & TF_INOSPACE) == 0 && ((flags & TF_EPSPACE) == 0 || (nextflags & TF_IPSPACE) == 0)) {
		inputs[index].type = INPUT_KEYBOARD;
		inputs[index].ki.wVk = VK_SPACE;

		inputs[index + 1].type = INPUT_KEYBOARD;
		inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
		inputs[index + 1].ki.wVk = VK_SPACE;

		finaltext += ' ';
		index += 2;
	}

	if (finaltext.length() == 0) {
		flags = flags | prevflags | TF_INOSPACE;
	}


	int totalkeys = index - 2;
	if (spaceinfront)
		totalkeys = index;

	INPUT* keys = &(inputs[2]);
	if (spaceinfront) {
		keys = inputs;
	}

	if (!redirect) {
		int sent = 0;
		while (sent < totalkeys) {
			sent += SendInput(totalkeys - sent, &(keys[sent]), sizeof(INPUT));
		}
	}

	delete inputs;

	return finaltext;
}

void deleteN(int n) {
	INPUT* inputs = new INPUT[n * 2];
	memset(inputs, 0, sizeof(INPUT)* n * 2);

	for (int i = 0; i < n; i++) {
		inputs[i * 2].type = INPUT_KEYBOARD;
		inputs[i * 2].ki.wVk = VK_BACK;

		inputs[i * 2 + 1].type = INPUT_KEYBOARD;
		inputs[i * 2 + 1].ki.dwFlags = KEYEVENTF_KEYUP;
		inputs[i * 2 + 1].ki.wVk = VK_BACK;
	}

	int sent = 0;
	while (sent < n * 2) {
		sent += SendInput((n * 2) - sent, &(inputs[sent]), sizeof(INPUT));
	}
}

void spaceN(int n) {
	INPUT* inputs = new INPUT[n * 2];
	memset(inputs, 0, sizeof(INPUT)* n * 2);

	for (int i = 0; i < n; i++) {
		inputs[i * 2].type = INPUT_KEYBOARD;
		inputs[i * 2].ki.wVk = VK_SPACE;

		inputs[i * 2 + 1].type = INPUT_KEYBOARD;
		inputs[i * 2 + 1].ki.dwFlags = KEYEVENTF_KEYUP;
		inputs[i * 2 + 1].ki.wVk = VK_SPACE;
	}

	int sent = 0;
	while (sent < n * 2) {
		sent += SendInput((n * 2) - sent, &(inputs[sent]), sizeof(INPUT));
	}
}

int delnP(std::list<singlestroke*>::iterator max, std::list<singlestroke*>::iterator &min, const std::list<singlestroke*>::const_iterator &end, int &addspaces, int &rstrokes) {
	if (max == min) {
		return 0;
	}
	std::list<singlestroke*>::iterator temp = max;
	int t = 0;

	while (max != min) {
		if ((*max)->textout->first == (*max))
			t += (*max)->textout->text.length();
		rstrokes++;
		max++;
	}

	if (settings.space == 1) {
		if (((*temp)->textout->flags & TF_INOSPACE) == TF_INOSPACE) {
			temp = max;
			if (temp == end) {
				//
			}
			else if (((*temp)->textout->flags & TF_ENOSPACE) == TF_ENOSPACE) {
				//
			}
			else {
				addspaces = 1;
			}
		}
		else if (((*temp)->textout->flags & TF_IPSPACE) == TF_IPSPACE) {
			temp = max;
			if (temp == end) {
				//
			}
			else if (((*temp)->textout->flags & TF_ENOSPACE) == TF_ENOSPACE) {
				//return t;
			}
			else  if (((*temp)->textout->flags & TF_EPSPACE) == TF_EPSPACE) {
				addspaces = 1;
				//return t;
			}
		}
	}

	return t;
}

int deln(std::list<singlestroke*>::iterator &i, const std::list<singlestroke*>::const_iterator &end, int &addspaces, int &rstrokes) {
	std::list<singlestroke*>::iterator temp = i;
	while (i != end) {
		if ((*i)->textout->first != (*temp))
			break;
		rstrokes++;
		i++;
	}

	int t = (*temp)->textout->text.length();

	if (settings.space == 1) {
		if (((*temp)->textout->flags & TF_INOSPACE) == TF_INOSPACE) {
			temp = i;
			if (temp == end) {
				//
			}
			else if (((*temp)->textout->flags & TF_ENOSPACE) == TF_ENOSPACE) {
				//
			}
			else {
				addspaces = 1;
			}
		}
		else if (((*temp)->textout->flags & TF_IPSPACE) == TF_IPSPACE) {
			temp = i;
			if (temp == end) {
				//
			}
			else if (((*temp)->textout->flags & TF_ENOSPACE) == TF_ENOSPACE) {
				//return t;
			}
			else  if (((*temp)->textout->flags & TF_EPSPACE) == TF_EPSPACE) {
				addspaces = 1;
				//return t;
			}
		}
	}

	return t;
}

void findanentry(unsigned __int8* stroke, dictionary * d, std::list<singlestroke*>::const_iterator li, std::list<singlestroke*>::const_iterator le, std::string &text, int &length) {
	if (d->longest <= 0)
		return;

	unsigned __int8* sbuffer = new unsigned __int8[d->longest*3];
	int index = d->longest - 1;

	sbuffer[index*3] = stroke[0];
	sbuffer[index*3 + 1] = stroke[1];
	sbuffer[index*3 + 2] = stroke[2];

	DB_TXN* trans;
	d->env->txn_begin(d->env, NULL, &trans, DB_READ_COMMITTED | DB_TXN_NOWAIT);


	DB_TXN* ptrans = NULL;
	if (projectdata.open) {
		projectdata.d->env->txn_begin(projectdata.d->env, NULL, &ptrans, DB_READ_COMMITTED | DB_TXN_NOWAIT);
		ptrans->set_priority(ptrans, 200);
	}

	trans->set_priority(trans, 200);
	//trans->set_timeout(trans, 1000, DB_SET_LOCK_TIMEOUT);
	if (projectdata.open) {
		if (projectdata.d->findDItem(&(sbuffer[index * 3]), 3, text, ptrans)) {
			length = 1;
		}
		else if (d->findDItem(&(sbuffer[index * 3]), 3, text, trans)) {
			length = 1;
		}
	}
	else {
		if (d->findDItem(&(sbuffer[index * 3]), 3, text, trans)) {
			length = 1;
		}
	}

	index--;

	while (li != le && index >= 0) {
		//sbuffer[0] = 3 + 3 * index;
		sbuffer[index * 3] = (*li)->value.ival[0];
		sbuffer[index * 3 + 1] = (*li)->value.ival[1];
		sbuffer[index * 3 + 2] = (*li)->value.ival[2];


		if (projectdata.open) {
			if (projectdata.d->findDItem(&(sbuffer[index * 3]), (d->longest - index) * 3, text, ptrans)) {
				length = (d->longest - index);
			}
			else if (d->findDItem(&(sbuffer[index * 3]), (d->longest - index) * 3, text, trans)) {
				length = (d->longest - index);
			}
		}
		else {
			if (d->findDItem(&(sbuffer[index * 3]), (d->longest - index) * 3, text, trans)) {
				length = (d->longest - index);
			}
		}


		index--;
		li++;
	}


	trans->commit(trans, 0);

	if (projectdata.open)
		ptrans->commit(ptrans, 0);

	delete sbuffer;
}

void deleteandspace(const int& del, const int &space) {
	if (inputstate.redirect != NULL) {
		tstring wtxt = getWinStr(inputstate.redirect);
		for (int i = 0; i < del; i++) {
			wtxt.pop_back();
		}
		for (int i = 0; i < space; i++) {
			wtxt += TEXT(' ');
		}
		SetWindowText(inputstate.redirect, wtxt.c_str());
	}
	else if (projectdata.open) {
		projectdata.settingsel = true;
		tstring wtxt;
		CHARRANGE crng;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXGETSEL, NULL, (LPARAM)&crng);
		crng.cpMin -= del;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crng);

		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_REPLACESEL, FALSE, (LPARAM)TEXT(""));

		//crng.cpMin += space;
		//crng.cpMax = crng.cpMin;
		//SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crng);
		projectdata.settingsel = false;
	} 
	else {
		deleteN(del);
		spaceN(space);
	}
}

void deletess(singlestroke* t) {
	if (t->textout->first == t) {
		delete t->textout;
	}
	delete t;
}

void deletelist(std::list<singlestroke*> temp, std::list<singlestroke*>::iterator &insert, std::list<singlestroke*> * target) {
	std::list<singlestroke*>::iterator ti = temp.end();
	if (temp.size() > 0) {
		ti--;
		for (; ti != temp.begin(); ti--) {
			InnerProcess((*ti)->value.ival, insert, target);
			deletess((*ti));
		}
		InnerProcess((*ti)->value.ival, insert, target);
		deletess((*ti));
	}
}

void sendstandard(const tstring& txt, singlestroke* s, std::list<singlestroke*>::iterator insert, std::list<singlestroke*> * list, std::list<singlestroke*>::iterator* end = NULL) {
	unsigned __int8 tflags = 0;
	unsigned __int8 nflags = 0;
	
	if (insert != list->cend()) {
		tflags = (*insert)->textout->flags;
	}
	
	auto icpy = insert;
	if (end != NULL)
		icpy = *end;
	if (icpy != list->cbegin() && list->size() != 0) {
		icpy--;
		nflags = (*icpy)->textout->flags;
	}

	int t = settings.space;
	if (projectdata.open && inputstate.redirect == NULL)
		settings.space = 2;

	BOOL prependspace = FALSE;
	
	s->textout->text = sendText(txt, s->textout->flags, tflags, nflags, prependspace, inputstate.redirect != NULL || projectdata.open);

	if (prependspace == TRUE) {
		if (insert != list->cend()) {
			(*insert)->textout->text.pop_back();
		}
	}
	
	settings.space = t;

	if (inputstate.redirect != NULL) {
		tstring wtxt = getWinStr(inputstate.redirect);
		wtxt += s->textout->text;
		SetWindowText(inputstate.redirect, wtxt.c_str());
	}
	else if (projectdata.open) {
		projectdata.settingsel = true;
		CHARRANGE crng;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXGETSEL, NULL, (LPARAM)&crng);
		crng.cpMin = crng.cpMax;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crng);

		if (settings.space != 2) {

			bool wantssbefore = (s->textout->flags & TF_INOSPACE) == 0 && (tflags & TF_ENOSPACE) == 0 && ((s->textout->flags & TF_IPSPACE) == 0 || (tflags & TF_EPSPACE) == 0);
			if (!spacebefore(insert, list, false) && wantssbefore) {
				s->textout->text = TEXT(' ') + s->textout->text;
			}

			if (end != NULL) {
				insert = *end;
			}

			bool wantssafter = (s->textout->flags & TF_ENOSPACE) == 0 && (nflags & TF_INOSPACE) == 0 && ((s->textout->flags & TF_EPSPACE) == 0 || (nflags & TF_IPSPACE) == 0);
			if (!spaceafter(insert, list, false) && wantssafter) {
				s->textout->text += TEXT(' ');
			}

		}
		
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_REPLACESEL, FALSE, (LPARAM)(s->textout->text.c_str()));
		projectdata.settingsel = false;
	}
}



void InnerProcess(unsigned __int8* stroke, std::list<singlestroke*>::iterator &insert, std::list<singlestroke*> * target) {
	int longest = 0;

	std::string ilongstemp;
	findanentry(stroke, sharedData.currentd, insert, target->cend(), ilongstemp, longest);
	tstring ilongs = strtotstr(ilongstemp);

	// is this a predefined suffix?
	if (longest == 0 && insert != target->cend()) {
		union {
			unsigned __int8 sval[4];
			unsigned __int32 ival;
		} tstroke;
		for (auto it = sharedData.currentd->suffix.cbegin(); it != sharedData.currentd->suffix.cend(); it++) {
			tstroke.ival = (*it).first;
			if (stroke[0] == tstroke.sval[0] && stroke[1] == tstroke.sval[1] && stroke[2] == tstroke.sval[2]) {
				if ((*insert)->textout->text.length() >= 3) {
					textoutput *tx = (*insert)->textout;
					bool poppedspace = false;

					int oldlen = tx->text.length();
					if (tx->text[0] == TEXT(' ')) {
						tx->text.erase(tx->text.begin());
					}
					if (tx->text[tx->text.length() - 1] == TEXT(' ')) {
						tx->text.pop_back();
						poppedspace = true;
					}
					addending(tx->text, (*it).second);
					//if (poppedspace)
					//	tx->text += TEXT(' ');

					deleteandspace(oldlen, 0);

					singlestroke* s = new singlestroke(stroke);
					s->textout = tx;
					tx->first = s;
					auto itemp = insert;
					itemp++;
					while (itemp != target->cend()) {
						if ((*itemp)->textout->first == (*itemp))
							break;
						itemp++;
					}

					sendstandard(tx->text, s, itemp, target, &insert);

					insert = target->insert(insert, s);
					trimStokesList();
					return;
				}
			}
		}
	}

	//is a predefined suffix folded into this stroke?
	if (longest == 0) {
		union {
			unsigned __int8 sval[4];
			unsigned __int32 ival;
		} tstroke;
		unsigned __int8 stemp[4];
		memset(stemp, 0, 4);
		for (auto it = sharedData.currentd->suffix.cbegin(); longest == 0 && it != sharedData.currentd->suffix.cend(); it++) {
			tstroke.ival = (*it).first;
			if ((stroke[0] & tstroke.sval[0]) == tstroke.sval[0] && (stroke[1] & tstroke.sval[1]) == tstroke.sval[1] && (stroke[2] & tstroke.sval[2]) == tstroke.sval[2]) {
				for (int i = 0; i < 3; i++)
					stemp[i] = stroke[i] & ~(tstroke.sval[i]);
				findanentry(stemp, sharedData.currentd, insert, target->cend(), ilongstemp, longest);
				ilongs = strtotstr(ilongstemp);
				if (longest != 0)
					addending(ilongs, (*it).second);
			}
		}
	}

	//either mistraslate or number
	if (longest == 0) {
		ilongs = TEXT("");
		bool number = false;

		if ((stroke[0] & (sharedData.number[0] | sharedData.currentd->number[0])) == stroke[0] &&
			(stroke[1] & (sharedData.number[1] | sharedData.currentd->number[1])) == stroke[1] &&
			(stroke[2] & (sharedData.number[2] | sharedData.currentd->number[2])) == stroke[2] &&
			(stroke[2] & 0x40) != 0) {

			bool special = ((stroke[0] & sharedData.currentd->number[0]) == sharedData.currentd->number[0]) &&
				((stroke[1] & sharedData.currentd->number[1]) == sharedData.currentd->number[1]) &&
				((stroke[2] & sharedData.currentd->number[2]) == sharedData.currentd->number[2]);

			number = true;
			stroke[0] &= ~sharedData.currentd->number[0];
			stroke[1] &= ~sharedData.currentd->number[1];
			stroke[2] &= ~sharedData.currentd->number[2];

			stroketocsteno(stroke, ilongs, sharedData.currentd->format, true);
			if (special) {
				if (ilongs.length() == 1) {
					ilongs += ilongs;
				}
				else {
					std::reverse(ilongs.begin(), ilongs.end());
				}
			}

			ilongs = TEXT("&") + ilongs + TEXT("&");
		}
		else {
			if (settings.mistrans == FALSE)
				stroketocsteno(stroke, ilongs, sharedData.currentd->format);
		}

		longest = 1;
	}

	// this is how strokes get sent and old data erased

	singlestroke* s = new singlestroke(stroke);

	s->textout = new textoutput();

	s->textout->first = s;

	std::list<singlestroke*> temp;
	std::list<singlestroke*> newword;
	std::list<singlestroke*>::iterator deli = insert;
	int sremoved = 0;
	int spacestoadd = 0;
	int charstodelete = 0;

	while (sremoved + 1 < longest) {
		spacestoadd = 0;
		charstodelete += deln(deli, target->end(), spacestoadd, sremoved);
	}
	if (projectdata.open && inputstate.redirect == NULL)
		spacestoadd = 0;
	//process

	deleteandspace(charstodelete, spacestoadd);

	if (spacestoadd != 0 && deli != target->end()) {
		(*deli)->textout->text += TEXT(' ');
	}

	for (int i = 0; i < longest - 1; i++) {
		newword.push_front((*insert));
		insert = target->erase(insert);
	}

	temp.splice(temp.end(), *target, insert, deli);
	deletelist(temp, deli, target);

	sendstandard(ilongs, s, deli, target);

	for (auto ti = newword.begin(); ti != newword.cend(); ti++) {
		if ((*ti)->textout->first == *ti) {
			delete (*ti)->textout;
		}
		(*ti)->textout = s->textout;
		deli = target->insert(deli, (*ti));
	}

	//MessageBox(NULL, s->textout->text.c_str(), TEXT("A"), MB_OK);

	insert = target->insert(deli, s);
	trimStokesList();
}

void freeRange(std::list<singlestroke*>::iterator max, std::list<singlestroke*>::iterator &min) {
	while (max != min) {
		deletess(*max);
		max++;
	}
}

bool spacebefore(std::list<singlestroke*>::iterator it, std::list<singlestroke*>* target, bool erase) {
	while (it != target->cend()) {
		if ((*it)->textout->text.length() > 0)
			break;
		if (((*it)->textout->flags & TF_ENOSPACE) != 0)
			return false;
		it++;
	}
	if (it != target->cend()) {
		if ((*it)->textout->text[(*it)->textout->text.length() - 1] == TEXT(' ')) {
			if (erase)
				(*it)->textout->text.pop_back();
			return true;
		}
		return false;
	}
	else {
		return true;
	}
}

bool spaceafter(std::list<singlestroke*>::iterator it, std::list<singlestroke*>* target, bool erase) {
	if (it == target->cbegin()) {
		return true;
	} 

	--it;
	if (((*it)->textout->flags & TF_INOSPACE) != 0)
		return false;
	

	while ((*it)->textout->text.length() == 0) {
		if (it == target->cbegin())
			return true;
		--it;
		if (((*it)->textout->flags & TF_INOSPACE) != 0)
			return false;
		}

	if ((*it)->textout->text[0] == TEXT(' ')) {
		if (erase)
			(*it)->textout->text.erase((*it)->textout->text.begin());
		return true;
	}

	return false;
}

void processSingleStroke(unsigned __int8* stroke) {

	if (newwordwin.running) {
		if (compare3(stroke, sharedData.currentd->stab)) {
			NewDlgNextFocus();
			return;
		}
		if (compare3(stroke, sharedData.currentd->sreturn)) {
			if (newwordwin.focusedcontrol == 0) {
				NewDlgNextFocus();
			}
			else if (newwordwin.focusedcontrol == 1 || newwordwin.focusedcontrol == 2) {
				PostMessage(newwordwin.dlgwnd, WM_COMMAND, IDOK, NULL);
			}
			else if (newwordwin.focusedcontrol == 3) {
				PostMessage(newwordwin.dlgwnd, WM_COMMAND, IDCANCEL, NULL);
			}
			return;
		}
	}

	if (projectdata.open && projectdata.addingnew) {
		if (compare3(stroke, sharedData.currentd->stab)) {
			PViewNextFocus();
			return;
		}
		if (compare3(stroke, sharedData.currentd->sreturn)) {
			if (projectdata.focusedcontrol == 0) {
				PViewNextFocus();
			}
			else if (projectdata.focusedcontrol == 1 || projectdata.focusedcontrol == 2) {
				PostMessage(projectdata.dlg, WM_COMMAND, IDC_POK, NULL);
			}
			else if (projectdata.focusedcontrol == 3) {
				PostMessage(projectdata.dlg, WM_COMMAND, IDC_PCANCEL, NULL);
			}
			return;
		}
	}

	if (inputstate.redirect != NULL && inputstate.sendasstrokes) {
		tstring wntxt = getWinStr(inputstate.redirect);

		if (compare3(stroke, sharedData.currentd->sdelete)) {
			while (wntxt.length() > 0) {
				if (wntxt.back() == TEXT('/')) {
					wntxt.pop_back();
					break;
				}
				wntxt.pop_back();
			}
			SetWindowText(inputstate.redirect, wntxt.c_str());
		}
		else {
			if (wntxt.length() == 0) {
				stroketocsteno(stroke, wntxt, sharedData.currentd->format);
				SetWindowText(inputstate.redirect, wntxt.c_str());
			}
			else {
				wntxt += TEXT('/');
				stroketocsteno(stroke, wntxt, sharedData.currentd->format);
				SetWindowText(inputstate.redirect, wntxt.c_str());
			}
		}
		return;
	}

	std::list<singlestroke*>* target = &sharedData.strokes;
	std::list<singlestroke*>::iterator insert = sharedData.strokes.begin();
	bool multidelete = false;

	if (inputstate.redirect == NULL && projectdata.open) {
		target = &projectdata.strokes;
		projectdata.settingsel = true;

		CHARRANGE crng;

		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXGETSEL, NULL, (LPARAM)&crng);

		auto max = GetItemByText(crng.cpMax);
		auto min = GetItemByText(crng.cpMin);
		multidelete = max != min;

		bool nostroke = compare3(stroke, sharedData.currentd->spaste) || compare3(stroke, sharedData.currentd->scut) || compare3(stroke, sharedData.currentd->scopy) || compare3(stroke, sharedData.currentd->sdelete)
			|| compare3(stroke, sharedData.currentd->sleft) || compare3(stroke, sharedData.currentd->sright) || compare3(stroke, sharedData.currentd->sshleft) || compare3(stroke, sharedData.currentd->sshright)
			|| compare3(stroke, sharedData.currentd->sreprocess);
		bool nodelete = compare3(stroke, sharedData.currentd->scopy) || compare3(stroke, sharedData.currentd->sleft) || compare3(stroke, sharedData.currentd->sright) || compare3(stroke, sharedData.currentd->sshleft) || compare3(stroke, sharedData.currentd->sshright);


		CHARRANGE crngb;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXGETSEL, NULL, (LPARAM)&crngb);
		int line = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, crngb.cpMin);
		int lineb = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, crngb.cpMax);

		if (compare3(stroke, sharedData.currentd->scut) || compare3(stroke, sharedData.currentd->scopy) || compare3(stroke, sharedData.currentd->sreprocess)) {
			freeRange(projectdata.clipboard.begin(), projectdata.clipboard.end());
			projectdata.clipboard.clear();
			for (auto i = max; i != min; i++) {
				projectdata.clipboard.push_front(*i);
			}
		}

		if (nostroke) {
			if (nodelete) {
			}
			else {
				if (!multidelete && compare3(stroke, sharedData.currentd->sdelete)) {
					RegisterDelete(line - 1);

					if (line != 0)
						crngb.cpMin = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, line - 1, 0) + 23;
					else
						crngb.cpMin = 23;
					SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, NULL, (LPARAM)&crngb);
				}
				else {
					for (int i = lineb - 1; i >= line; i--)
						RegisterDelete(i);
				}
				SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_REPLACESEL, FALSE, (LPARAM)TEXT(""));
			}
		}
		else {
			for (int i = lineb-1; i >= line; i--)
				RegisterDelete(i);
			RegisterStroke(stroke, line);

			TCHAR buffer[32] = TEXT("\r\n");
			stroketosteno(stroke, &buffer[2], sharedData.currentd->format);
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_REPLACESEL, FALSE, (LPARAM)buffer);
		}


		int spacestoadd = 0;
		int sremoved = 0;
		int charstodelete = 0;

		if (!nodelete) {
			charstodelete = delnP(max, min, projectdata.strokes.cend(), spacestoadd, sremoved);

			if (settings.space != 2) {
				bool altered = false;

				if (spaceafter(max, target, true)) {
					crng.cpMax++;
					altered = true;
				}

				if (spacebefore(min, target, true)) {
					if (crng.cpMin != 0)
						crng.cpMin--;
					altered = true;
				}

				if (altered)
					SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crng);
			}



			SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_REPLACESEL, FALSE, (LPARAM)TEXT(""));
			if (!compare3(stroke, sharedData.currentd->scut) && !compare3(stroke, sharedData.currentd->sreprocess))
				freeRange(max, min);

			target->erase(max, min);
		}
		insert = min;
		projectdata.settingsel = false;
	}

	if ((compare3(stroke, sharedData.currentd->spaste) || compare3(stroke, sharedData.currentd->sreprocess)) && inputstate.redirect == NULL && projectdata.open) {

		CHARRANGE crngb;

		for (auto i = projectdata.clipboard.cbegin(); i != projectdata.clipboard.cend(); i++) {
			InnerProcess((*i)->value.ival, insert, target);
			if (!compare3(stroke, sharedData.currentd->sreprocess)) {
				TCHAR buffer[32] = TEXT("\r\n");
				stroketosteno((*i)->value.ival, &buffer[2], sharedData.currentd->format);

				SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXGETSEL, NULL, (LPARAM)&crngb);
				int line = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, crngb.cpMin);
				RegisterStroke((*i)->value.ival, line);

				SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_REPLACESEL, FALSE, (LPARAM)buffer);
				
				SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXGETSEL, NULL, (LPARAM)&crngb);
				crngb.cpMin = crngb.cpMax;
				SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crngb);
			}
		}
		return;
	}
	else if (compare3(stroke, sharedData.currentd->scopy) && inputstate.redirect == NULL && projectdata.open) {
		return;
	}
	else if (compare3(stroke, sharedData.currentd->scut) && inputstate.redirect == NULL && projectdata.open) {
		if (insert != target->cend() && insert != target->cbegin()) {
			auto prev = insert;
			--prev;
			if (((*prev)->textout->flags & TF_INOSPACE) == 0 && ((*insert)->textout->flags & TF_ENOSPACE) == 0 && (((*prev)->textout->flags & TF_IPSPACE) == 0 || ((*insert)->textout->flags & TF_EPSPACE) == 0)) {
				if (!spacebefore(insert, target, false) && !spaceafter(insert, target, false) && settings.space != 2) {
					(*insert)->textout->text += TEXT(' ');
					SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_REPLACESEL, FALSE, (LPARAM)TEXT(" "));
				}

			}
		}
	}
	else if (compare3(stroke, sharedData.currentd->sleft) && inputstate.redirect == NULL && projectdata.open){
		CHARRANGE crngb;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXGETSEL, NULL, (LPARAM)&crngb);
		int line = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, crngb.cpMin);
		auto it = GetItem(line);
		int sub = 0;
		while (it != target->cend()) {
			it++;
			sub++;
			if ((*it) == (*it)->textout->first) {
				break;
			}
		}

		int lineindex = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, line-sub, 0);

		crngb.cpMin = lineindex + 23;
		crngb.cpMax = lineindex + 23;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crngb);

		SetTextSel(line - sub, line - sub);
		return;
	}
	else if (compare3(stroke, sharedData.currentd->sshleft) && inputstate.redirect == NULL && projectdata.open){
		CHARRANGE crngb;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXGETSEL, NULL, (LPARAM)&crngb);
		int line = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, crngb.cpMin);
		int lineb = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, crngb.cpMax);

		auto it = GetItem(line);
		int sub = 0;
		while (it != target->cend()) {
			it++;
			sub++;
			if ((*it) == (*it)->textout->first) {
				break;
			}
		}

		int lineindex = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, line - sub, 0);

		crngb.cpMin = lineindex + 23;

		SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crngb);
		SetTextSel(line - sub, lineb);

		return;
	}
	else if (compare3(stroke, sharedData.currentd->sright) && inputstate.redirect == NULL && projectdata.open){
		CHARRANGE crngb;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXGETSEL, NULL, (LPARAM)&crngb);
		int line = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, crngb.cpMax);
		auto it = GetItem(line);
		int sub = 0;
		while (it != target->cbegin()) {
			it--;
			sub++;
			if ((*it) == (*it)->textout->first) {
				break;
			}
		}

		int lineindex = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, line + sub, 0);

		crngb.cpMin = lineindex + 23;
		crngb.cpMax = lineindex + 23;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crngb);
		SetTextSel(line + sub, line + sub);

		return;
	}
	else if (compare3(stroke, sharedData.currentd->sshright) && inputstate.redirect == NULL && projectdata.open){
		CHARRANGE crngb;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXGETSEL, NULL, (LPARAM)&crngb);
		int line = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, crngb.cpMax);
		int lineb = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, crngb.cpMin);

		auto it = GetItem(line);
		int sub = 0;
		while (it != target->cbegin()) {
			it--;
			sub++;
			if ((*it) == (*it)->textout->first) {
				break;
			}
		}

		int lineindex = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, line + sub, 0);

		crngb.cpMax = lineindex + 23;
		
		SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, 0, (LPARAM)&crngb);
		SetTextSel(lineb, line + sub);

		return;
	}
	else if (compare3(stroke, sharedData.currentd->sdelete)) {
		int sremoved = 0;
		int spacestoadd = 0;
		int charstodelete = 0;
		if (target->empty()) {
			return;
		}

		std::list<singlestroke*>::iterator deli = insert;

		if (!multidelete) {	
			charstodelete = deln(deli, target->end(), spacestoadd, sremoved);

			if (projectdata.open && inputstate.redirect == NULL)
				spacestoadd = 0;

			deleteandspace(charstodelete, spacestoadd);


			if (spacestoadd != 0 && deli != target->end()) {
				(*deli)->textout->text += TEXT(' ');
			}

			std::list<singlestroke*> temp;
			deletess(*insert);
			insert = target->erase(insert);

			temp.splice(temp.end(), *target, insert, deli);

			deletelist(temp, deli, target);
		}

	
		if (deli != target->cend() && deli != target->cbegin() && projectdata.open && inputstate.redirect == NULL) {
			auto prev = deli;
			--prev;
			if (((*prev)->textout->flags & TF_INOSPACE) == 0 && ((*deli)->textout->flags & TF_ENOSPACE) == 0 && (((*prev)->textout->flags & TF_IPSPACE) == 0 || ((*deli)->textout->flags & TF_EPSPACE) == 0)) {
				if (!spacebefore(deli, target, false) && !spaceafter(deli, target, false) && settings.space != 2) {
					(*deli)->textout->text += TEXT(' ');
					SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_REPLACESEL, FALSE, (LPARAM)TEXT(" "));
				}
				
			}
		}


		CHARRANGE crng;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXGETSEL, NULL, (LPARAM)&crng);
		crng.cpMin = crng.cpMax;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crng);

		return;
	}

	InnerProcess(stroke, insert, target);
}


DWORD WINAPI processStrokes(LPVOID lpParam)
{
	sdata* sharedData = (sdata*)lpParam;



	loadDictionaries();

	while (controls.inited != TRUE) {
		Sleep(1);
	}


	int len = SendMessage(controls.mestroke, WM_GETTEXTLENGTH, 0, 0);
	SendMessage(controls.mestroke, EM_SETSEL, len, len);
	SendMessage(controls.mestroke, EM_REPLACESEL, FALSE, (LPARAM)TEXT("\r\nLOADING FINISHED"));

	CreateThread(NULL, 0, searchDictionary, NULL, 0, NULL);

	while (sharedData->running == TRUE) {
		WaitForSingleObject(sharedData->protectqueue, INFINITE);
		if (!sharedData->inputs.empty()) {
			union {
				__int32 val;
				unsigned __int8 ival[4];
			} cstroke;

			cstroke.val = sharedData->inputs.front();
			sharedData->inputs.pop();
			ReleaseMutex(sharedData->protectqueue);

			if (sharedData->currentd != NULL) {
				processSingleStroke(cstroke.ival);

				sharedData->addedtext = TRUE;
				SetEvent(sharedData->newtext);
			}
		}
		else {
			ReleaseMutex(sharedData->protectqueue);
		}
		
		WaitForSingleObject(sharedData->newentry, INFINITE);
	}

	return 0;
}
