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

void addStroke(__int32 s) {
	WaitForSingleObject(sharedData.protectqueue, INFINITE);
	sharedData.inputs.push(s);
	ReleaseMutex(sharedData.protectqueue);
	SetEvent(sharedData.newentry);
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
			finaltext += *i;
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
			}
			else {
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
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
		flags = flags | prevflags;
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

		crng.cpMin += space;
		crng.cpMax = crng.cpMin;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crng);
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

void deletelist(std::list<singlestroke*> &temp) {
	std::list<singlestroke*>::iterator ti = temp.end();
	if (temp.size() > 0) {
		ti--;
		for (; ti != temp.begin(); ti--) {
			processSingleStroke((*ti)->value.ival);
			deletess((*ti));
		}
		processSingleStroke((*ti)->value.ival);
		deletess((*ti));
	}
}

void sendstandard(const tstring& txt, singlestroke* s, std::list<singlestroke*>::iterator insert, std::list<singlestroke*> * list, bool shortver = false) {
	unsigned __int8 tflags = 0;
	unsigned __int8 nflags = 0;
	if (shortver) {
		tflags = s->textout->flags;
	}
	else {
		if (insert != list->cend()) {
			tflags = (*insert)->textout->flags;
		}
	}

	if (insert != list->cbegin() && list->size() != 0) {
		auto icpy = insert;
		icpy--;
		nflags = (*icpy)->textout->flags;
	}

	int t = settings.space;
	if (projectdata.open && inputstate.redirect == NULL)
		settings.space = -1;

	BOOL prependspace = FALSE;
	if (shortver) {
		s->textout->text = sendText(txt, s->textout->flags, TF_ENOSPACE, nflags, prependspace, inputstate.redirect != NULL || projectdata.open);
		s->textout->flags = tflags;
	}
	else {
		s->textout->text = sendText(txt, s->textout->flags, tflags, nflags, prependspace, inputstate.redirect != NULL || projectdata.open);
		if (prependspace == TRUE) {
			if (insert != list->cend()) {
				(*insert)->textout->text.pop_back();
			}
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

		if (s->textout->text.length() > 0 && settings.space != 2) {
			if (insert != list->cend()) {
				if ((s->textout->flags & TF_INOSPACE) == 0 && ((*insert)->textout->flags & TF_ENOSPACE) == 0 && ((s->textout->flags & TF_IPSPACE) == 0 || ((*insert)->textout->flags & TF_EPSPACE) == 0)) {
					//should have beggining seperation
					if ((*insert)->textout->text.length() > 0 && s->textout->text[0] != TEXT(' ')) {
						if ((*insert)->textout->text[(*insert)->textout->text.length() - 1] != TEXT(' ')) {
							s->textout->text = TEXT(' ') + s->textout->text;
						}
					}
				}
			}
			if ((s->textout->flags & TF_ENOSPACE) == 0 && (nflags & TF_INOSPACE) == 0 && ((s->textout->flags & TF_EPSPACE) == 0 || (nflags & TF_IPSPACE) == 0)) {
				//should have end seperation
				if (insert != list->cbegin() && s->textout->text[s->textout->text.length()-1] != TEXT(' ')) {
					insert--;
					if ((*insert)->textout->text.length() > 0) {
						if ((*insert)->textout->text[0] != TEXT(' ')) {
							s->textout->text += TEXT(' ');
						}
					}
				}
			}
		}
		
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_REPLACESEL, FALSE, (LPARAM)(s->textout->text.c_str()));
		projectdata.settingsel = false;
	}
}

void standardcleanup(singlestroke* s, std::list<singlestroke*>::iterator &insert, std::list<singlestroke*> * list) {
	list->insert(insert, s);
	trimStokesList();
}

void processSingleStroke(unsigned __int8* stroke) {

	if (newwordwin.running) {
		if (stroke[0] == sharedData.currentd->stab[0] && stroke[1] == sharedData.currentd->stab[1] && stroke[2] == sharedData.currentd->stab[2]) {
			NewDlgNextFocus();
			return;
		}
		if (stroke[0] == sharedData.currentd->sreturn[0] && stroke[1] == sharedData.currentd->sreturn[1] && stroke[2] == sharedData.currentd->sreturn[2]) {
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
		if (stroke[0] == sharedData.currentd->stab[0] && stroke[1] == sharedData.currentd->stab[1] && stroke[2] == sharedData.currentd->stab[2]) {
			PViewNextFocus();
			return;
		}
		if (stroke[0] == sharedData.currentd->sreturn[0] && stroke[1] == sharedData.currentd->sreturn[1] && stroke[2] == sharedData.currentd->sreturn[2]) {
			if (projectdata.focusedcontrol == 0) {
				PViewNextFocus();
			}
			else if (newwordwin.focusedcontrol == 1 || newwordwin.focusedcontrol == 2) {
				PostMessage(projectdata.dlg, WM_COMMAND, IDC_POK, NULL);
			}
			else if (newwordwin.focusedcontrol == 3) {
				PostMessage(projectdata.dlg, WM_COMMAND, IDC_PCANCEL, NULL);
			}
			return;
		}
	}

	if (inputstate.redirect != NULL && inputstate.sendasstrokes) {
		tstring wntxt = getWinStr(inputstate.redirect);

		if (stroke[0] == sharedData.currentd->sdelete[0] && stroke[1] == sharedData.currentd->sdelete[1] && stroke[2] == sharedData.currentd->sdelete[2]) {
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

	if (inputstate.redirect == NULL && projectdata.open) {
		target = &projectdata.strokes;
		projectdata.settingsel = true;

		CHARRANGE crng;

		if (stroke[0] == sharedData.currentd->sdelete[0] && stroke[1] == sharedData.currentd->sdelete[1] && stroke[2] == sharedData.currentd->sdelete[2]) {
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXGETSEL, NULL, (LPARAM)&crng);
			int line = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXLINEFROMCHAR, 0, crng.cpMin);
			crng.cpMin = SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_LINEINDEX, line - 1, 0) + 23;
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_EXSETSEL, NULL, (LPARAM)&crng);
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_REPLACESEL, FALSE, (LPARAM)TEXT(""));
		}
		else {
			TCHAR buffer[32] = TEXT("\r\n");
			stroketosteno(stroke, &buffer[2], sharedData.currentd->format);
			SendMessage(GetDlgItem(projectdata.dlg, IDC_PSTROKELIST), EM_REPLACESEL, FALSE, (LPARAM)buffer);
		}


		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXGETSEL, NULL, (LPARAM)&crng);

		auto max = GetItemByText(crng.cpMax);
		auto min = GetItemByText(crng.cpMin);

		int spacestoadd = 0;
		int sremoved = 0;
		int charstodelete = 0;

		charstodelete = delnP(max, min, projectdata.strokes.cend(), spacestoadd, sremoved);

		if (max != target->cbegin() && settings.space == 0) {
			auto maxn = --max;
			max++;
			if ((*maxn)->textout->text.length() > 0) {
				if ((*maxn)->textout->text[0] == TEXT(' ')) {
					(*maxn)->textout->text.erase((*maxn)->textout->text.begin());
					AdjustTextStart(maxn, -1);
					crng.cpMax++;
					SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crng);
				}
			}
		}

		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_REPLACESEL, FALSE, (LPARAM)TEXT(""));
		crng.cpMin += spacestoadd;
		crng.cpMax = crng.cpMin;
		SendMessage(GetDlgItem(projectdata.dlg, IDC_MAINTEXT), EM_EXSETSEL, NULL, (LPARAM)&crng);

		AdjustTextStart(min, spacestoadd - sremoved);

		if (spacestoadd != 0 && min != projectdata.strokes.cend()) {
			(*min)->textout->text += TEXT(' ');
		}
		insert = min;
		projectdata.settingsel = false;
	}



	if (stroke[0] == sharedData.currentd->sdelete[0] && stroke[1] == sharedData.currentd->sdelete[1] && stroke[2] == sharedData.currentd->sdelete[2]) {
		int sremoved = 0;
		int spacestoadd = 0;
		int charstodelete = 0;
		if (target->empty()) {
			return;
		}
		std::list<singlestroke*>::iterator deli = insert;
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
		deletelist(temp);
		return;
	}

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

					if (projectdata.open && inputstate.redirect == NULL)
						sendstandard(tx->text, s, insert, target);
					else
						sendstandard(tx->text, s, insert, target, true);

					if (projectdata.open && inputstate.redirect == NULL)
						AdjustTextStart(insert, tx->text.length() - oldlen);

					standardcleanup(s, insert, target);
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
		if (projectdata.open && inputstate.redirect == NULL)
			s->textout = new indexedtext();
		else
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
		if (projectdata.open && inputstate.redirect == NULL)
			AdjustTextStart(deli, spacestoadd - charstodelete);

		if (spacestoadd != 0 && deli != target->end()) {
			(*deli)->textout->text += TEXT(' ');
		}

		for (int i = 0; i < longest - 1; i++) {
			newword.push_front((*insert));
			insert = target->erase(insert);
		}

		temp.splice(temp.end(), *target, insert, deli);
		deletelist(temp);

		sendstandard(ilongs, s, insert, target);


		if (projectdata.open && inputstate.redirect == NULL) {
			if (insert != target->cend()) {
				((indexedtext*)(s->textout))->startingindex = ((indexedtext*)((*insert)->textout))->startingindex + (*insert)->textout->text.length();
			}
			else {
				((indexedtext*)(s->textout))->startingindex = 0;
			}
			AdjustTextStart(insert, s->textout->text.length());
		}

		for (auto ti = newword.begin(); ti != newword.cend(); ti++) {
			if ((*ti)->textout->first == *ti) {
				delete (*ti)->textout;
			}
			(*ti)->textout = s->textout;
			insert = target->insert(insert, (*ti));
			//sharedData.strokes.push_front(*ti);
		}

		standardcleanup(s, insert, target);
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
