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

std::string sendText(std::string fulltext, unsigned __int8 &flags, unsigned __int8 prevflags, BOOL &trimspace, bool redirect) {
	flags = 0;
	std::string::const_iterator i = fulltext.cbegin();
	HKL locale = GetKeyboardLayout(GetCurrentThreadId());

	bool insubcommand = false;
	bool inescape = false;

	std::string finaltext("");

	INPUT* inputs = new INPUT[fulltext.length() * 4 + 4];
	memset(inputs, 0, sizeof(INPUT)*(fulltext.length() * 4 + 4));
	int index = 2;
	std::stack<WORD> subseqstack;

	bool spaceinfront = false;
	bool first = true;
	bool poped = false;

	while (i != fulltext.cend() ) {
		if (inescape) {
			if (*i == 'p') {
				inescape = false;
				insubcommand = true;

				if (!inputstate.textstack.empty()) {
					unsigned __int8 newflags = 0;
					std::string cmd = inputstate.textstack.top();
					inputstate.textstack.pop();
					std::string result = sendText(cmd, newflags, prevflags, trimspace, redirect);
					finaltext = result + finaltext;
					poped = true;
				}
			}
			else if (*i == 'A') {
				inescape = false;
				insubcommand = true;

				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = VK_MENU;

				subseqstack.push(VK_MENU);
				index++;

				if (i + 1 != fulltext.cend()) {
					if (*(i + 1) == '[') {
						i++;
					}
				}
			}
			else if (*i == 'C') {
				inescape = false;
				insubcommand = true;

				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = VK_CONTROL;

				subseqstack.push(VK_CONTROL);
				index++;

				if (i + 1 != fulltext.cend()) {
					if (*(i + 1) == '[') {
						i++;
					}
				}
			}
			else if (*i == 'S') {
				inescape = false;
				insubcommand = true;

				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = VK_SHIFT;

				subseqstack.push(VK_SHIFT);
				index++;

				if (i + 1 != fulltext.cend()) {
					if (*(i + 1) == '[') {
						i++;
					}
				}
			}
			else if (*i == '-') {
				flags |= TF_LOWNEXT;
				inescape = false;
			}
			else if (*i == '+') {
				flags |= TF_CAPNEXT;
				inescape = false;
			}
			else if (*i == '?') {
				PostMessage(controls.main, WM_NEWITEMDLG, 0, 0);
				inescape = false;
			}
			else if (*i == 't') {
				SHORT rval = VK_TAB;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;

				finaltext += '\t';
			}
			else if (*i == 'h') {
				SHORT rval = VK_LEFT;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == 'k') {
				SHORT rval = VK_UP;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == 'j') {
				SHORT rval = VK_DOWN;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == 'l') {
				SHORT rval = VK_RIGHT;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == 'n') {
				SHORT rval = VK_RETURN;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
				finaltext += '\n';
			}
			else if (*i == 'b') {
				SHORT rval = VK_BACK;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == 'x') {
				SHORT rval = VK_ESCAPE;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == 'd') {
				SHORT rval = VK_DELETE;
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.wVk = LOBYTE(rval);

				inputs[index + 1].type = INPUT_KEYBOARD;
				inputs[index + 1].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index + 1].ki.wVk = LOBYTE(rval);

				index += 2;
				inescape = false;
			}
			else if (*i == 'F') {
				SHORT rval = VK_F1;
				switch (std::atoi(getSubSeq(i, fulltext.cend()).c_str())){
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
			else if (*i == 'M') {
				int m = std::atoi(getSubSeq(i, fulltext.cend()).c_str());
				setMode(m);
				settings.mode = m;
				SendMessage(controls.inputs, CB_SETCURSEL, (WPARAM)m, (LPARAM)0);
				inescape = false;
			}
			else if (*i == 'D') {
				std::string dictname = getSubSeq(i, fulltext.cend());

				std::list<std::tuple<std::string, dictionary*>>::iterator di = sharedData.dicts.begin();
				while (di != sharedData.dicts.cend()) {
					if (std::get<0, std::string, dictionary*>(*di).compare(dictname) == 0) {
						settings.dict = dictname;
						setDictionary(std::get<1, std::string, dictionary*>(*di));
						TCHAR buffer[200];
						std::copy(dictname.cbegin(), dictname.cend(), buffer);
						buffer[dictname.length()] = 0;
						int sel = SendMessage(controls.dicts, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)(buffer));
						SendMessage(controls.dicts, CB_SETCURSEL, (WPARAM)sel, (LPARAM)0);
					}
					di++;
				}
				inescape = false;
			}
			else if (*i == 'P') {
				std::string command = getSubSeq(i, fulltext.cend());
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
			if (*i == ']') {
				inputs[index].type = INPUT_KEYBOARD;
				inputs[index].ki.dwFlags = KEYEVENTF_KEYUP;
				inputs[index].ki.wVk = subseqstack.top();

				index += 1;
				subseqstack.pop();
				if (subseqstack.empty()) {
					insubcommand = false;
				}
			}
			else if (*i == '\\') {
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
		else if (*i == '^') {
			if (i == fulltext.cbegin()) {
				flags = flags | TF_INOSPACE;
			}
			else {
				flags = flags | TF_ENOSPACE;
			}
		}
		else if (*i == '&') {
			if (i == fulltext.cbegin()) {
				flags = flags | TF_IPSPACE;
			}
			else {
				flags = flags | TF_EPSPACE;
			}
		}
		else if (*i == '\\') {
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

		finaltext = ' ' + finaltext;
		spaceinfront = true;
	}

	if ((flags & TF_ENOSPACE) != TF_ENOSPACE && settings.space == 1 && finaltext.length() > 0) {
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
		unsigned int sent = 0;
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

	unsigned int sent = 0;
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

	unsigned int sent = 0;
	while (sent < n * 2) {
		sent += SendInput((n * 2) - sent, &(inputs[sent]), sizeof(INPUT));
	}
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
			temp++;
			if (temp == end) {
				return t;
			}
			if (((*temp)->textout->flags & TF_ENOSPACE) == TF_ENOSPACE) {
				return t;
			}
			addspaces = 1;
			return t;
		}
		if (((*temp)->textout->flags & TF_IPSPACE) == TF_IPSPACE) {
			temp++;
			if (temp == end) {
				return t;
			}
			if (((*temp)->textout->flags & TF_ENOSPACE) == TF_ENOSPACE) {
				return t;
			}
			if (((*temp)->textout->flags & TF_EPSPACE) == TF_EPSPACE) {
				addspaces = 1;
				return t;
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

	trans->set_priority(trans, 200);
	//trans->set_timeout(trans, 1000, DB_SET_LOCK_TIMEOUT);
	

	if (d->findDItem(&(sbuffer[index * 3]), 3, text, trans)) {
		length = 1;
	}

	index--;

	while (li != le && index >= 0) {
		//sbuffer[0] = 3 + 3 * index;
		sbuffer[index * 3] = (*li)->value.ival[0];
		sbuffer[index * 3 + 1] = (*li)->value.ival[1];
		sbuffer[index * 3 + 2] = (*li)->value.ival[2];

		
		if (d->findDItem(&(sbuffer[index * 3]), (d->longest - index) * 3, text, trans)) {
			length = (d->longest-index);
		}
		

		index--;
		li++;
	}

	trans->commit(trans, 0);

	delete sbuffer;
}

void deleteandspace(const int& del, const int &space) {
	if (inputstate.redirect == NULL) {
		deleteN(del);
		spaceN(space);
	}
	else {
		tstring wtxt = getWinStr(inputstate.redirect);
		for (int i = 0; i < del; i++) {
			wtxt.pop_back();
		}
		for (int i = 0; i < space; i++) {
			wtxt += TEXT(' ');
		}
		SetWindowText(inputstate.redirect, wtxt.c_str());
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

void sendstandard(const std::string& txt, singlestroke* s) {
	unsigned __int8 tflags = 0;
	//output text
	if (sharedData.strokes.size() != 0) {
		tflags = (*(sharedData.strokes.begin()))->textout->flags;
	}

	BOOL prependspace = FALSE;
	s->textout->text = sendText(txt, s->textout->flags, tflags, prependspace, inputstate.redirect != NULL);
	if (prependspace == TRUE) {
		if (sharedData.strokes.size() != 0) {
			(*(sharedData.strokes.begin()))->textout->text.pop_back();
		}
	}

	if (inputstate.redirect != NULL) {
		tstring wtxt = getWinStr(inputstate.redirect);
		wtxt += strtotstr(s->textout->text);
		SetWindowText(inputstate.redirect, wtxt.c_str());
	}
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

	if (stroke[0] == sharedData.currentd->sdelete[0] && stroke[1] == sharedData.currentd->sdelete[1] && stroke[2] == sharedData.currentd->sdelete[2]) {
		int sremoved = 0;
		int spacestoadd = 0;
		int charstodelete = 0;
		if (sharedData.strokes.empty()) {
			return;
		}
		std::list<singlestroke*>::iterator deli = sharedData.strokes.begin();
		charstodelete = deln(deli, sharedData.strokes.end(), spacestoadd, sremoved);
		deleteandspace(charstodelete, spacestoadd);

		if (spacestoadd != 0 && deli != sharedData.strokes.end()) {
			(*deli)->textout->text += ' ';
		}

		std::list<singlestroke*> temp;
		deletess(sharedData.strokes.front());
		sharedData.strokes.pop_front();
		temp.splice(temp.end(), sharedData.strokes, sharedData.strokes.begin(), deli);
		deletelist(temp);
		return;
	}

	int longest = 0;
	std::string ilongs;
	findanentry(stroke, sharedData.currentd, sharedData.strokes.cbegin(), sharedData.strokes.cend(), ilongs, longest);

	// is this a predefined suffix?
	if (longest == 0 && sharedData.strokes.cbegin() != sharedData.strokes.cend()) {
		union {
			unsigned __int8 sval[4];
			unsigned __int32 ival;
		} tstroke;
		for (auto it = sharedData.currentd->suffix.cbegin(); it != sharedData.currentd->suffix.cend(); it++) {
			tstroke.ival = (*it).first;
			if (stroke[0] == tstroke.sval[0] && stroke[1] == tstroke.sval[1] && stroke[2] == tstroke.sval[2]) {
				if (sharedData.strokes.front()->textout->text.length() >= 3) {
					textoutput *tx = sharedData.strokes.front()->textout;
					bool poppedspace = false;

					int oldlen = tx->text.length();
					if (tx->text[tx->text.length() - 1] == ' ') {
						tx->text.pop_back();
						poppedspace = true;
					}
					addending(tx->text, (*it).second);
					if (poppedspace)
						tx->text += ' ';

					deleteandspace(oldlen, 0);

					singlestroke* s = new singlestroke(stroke);
					s->textout = tx;
					tx->first = s;

					unsigned __int8 tflags = tx->flags;
					BOOL prependspace = FALSE;
					tx->text = sendText(tx->text, tx->flags, TF_ENOSPACE, prependspace, inputstate.redirect != NULL);
					tx->flags = tflags;

					if (inputstate.redirect != NULL) {
						tstring wtxt = getWinStr(inputstate.redirect);
						wtxt += strtotstr(s->textout->text);
						SetWindowText(inputstate.redirect, wtxt.c_str());
					}

					sharedData.strokes.push_front(s);
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
				findanentry(stemp, sharedData.currentd, sharedData.strokes.cbegin(), sharedData.strokes.cend(), ilongs, longest);
				if (longest != 0)
					addending(ilongs, (*it).second);
			}
		}
	}

	if (longest > 1) {
		//need to reprocess it
		singlestroke* s = new singlestroke(stroke);
		s->textout = new textoutput();
		s->textout->first = s;

		std::list<singlestroke*> temp;
		std::list<singlestroke*> newword;
		std::list<singlestroke*>::iterator deli = sharedData.strokes.begin();
		int sremoved = 0;
		int spacestoadd = 0;
		int charstodelete = 0;

		while (sremoved + 1 < longest) {
			spacestoadd = 0;
			charstodelete += deln(deli, sharedData.strokes.end(), spacestoadd, sremoved);
		}
		//process

		deleteandspace(charstodelete, spacestoadd);

		if (spacestoadd != 0 && deli != sharedData.strokes.end()) {
			(*deli)->textout->text += ' ';
		}

		for (int i = 0; i < longest - 1; i++) {
			newword.push_front(sharedData.strokes.front());
			sharedData.strokes.pop_front();
		}

		temp.splice(temp.end(), sharedData.strokes, sharedData.strokes.begin(), deli);
		deletelist(temp);

		sendstandard(ilongs, s);

		for (auto ti = newword.begin(); ti != newword.cend(); ti++) {
			if ((*ti)->textout->first == *ti) {
				delete (*ti)->textout;
			}
			(*ti)->textout = s->textout;
			sharedData.strokes.push_front(*ti);
		}

		sharedData.strokes.push_front(s);
		trimStokesList();
	}
	else if (longest == 1) {
		//just add
		singlestroke* s = new singlestroke(stroke);
		s->textout = new textoutput();
		s->textout->first = s;

		sendstandard(ilongs, s);

		//remove extras
		sharedData.strokes.push_front(s);
		trimStokesList();
	}
	else {
		//stroke not found, possibly #
		singlestroke* s = new singlestroke(stroke);
		s->textout = new textoutput();
		s->textout->first = s;

		//test for number
		std::string data("");
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

			stroketocsteno(stroke, data, sharedData.currentd->format, true);
			if (special) {
				if (data.length() == 1) {
					data += data;
				}
				else {
					std::reverse(data.begin(), data.end());
				}
			}

			data = "&" + data + "&";
		}
		else {
			stroketocsteno(stroke, data, sharedData.currentd->format);
		}

		


		if (number || settings.mistrans == FALSE) {
			sendstandard(data, s);
		}
		else {
			s->textout->text = std::string("");
			if (sharedData.strokes.size() != 0) {
				s->textout->flags = (*(sharedData.strokes.begin()))->textout->flags;
			}
			else {
				s->textout->flags = 0;
			}
		}

		sharedData.strokes.push_front(s);
		trimStokesList();
	}
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
