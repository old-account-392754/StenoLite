#include "stdafx.h"
#include "addendings.h"

#include <string>

bool isvowel(char c) {
	switch (c) {
	case 'a':
	case 'i':
	case 'o':
	case 'e':
	case 'u':
	case 'A':
	case 'E':
	case 'I':
	case 'O':
	case 'U':
		return true;
	default:
		return false;
	}
}

char* suftostr(const int& val) {
	switch (val) {
	case ENDING_S: return "s";
	case ENDING_IVE: return "ive";
	case ENDING_ION: return "ion";
	case ENDING_IONS: return "ions";
	case ENDING_TH: return "th";
	case ENDING_LY: return "ly";
	case ENDING_ING: return "ing";
	case ENDING_INGS: return "ings";
	case ENDING_ED: return "ed";
	case ENDING_ST: return "st";
	case ENDING_ER: return "er";
	case ENDING_ERS: return "ers";
	case ENDING_NESS: return "ness";
	case ENDING_ABLE: return "able";
	case ENDING_MENT: return "ment";
	case ENDING_FUL: return "ful";
	case ENDING_IST: return "ist";
	case ENDING_ISTS: return "ists";
	default: return "";
	}
}

int strtosuf(const std::string& source) {
	if (source.compare("s") == 0) {
		return ENDING_S;
	}
	else if (source.compare("ive") == 0) {
		return ENDING_IVE;
	}
	else if (source.compare("ion") == 0) {
		return ENDING_ION;
	}
	else if (source.compare("ions") == 0) {
		return ENDING_IONS;
	}
	else if (source.compare("th") == 0) {
		return ENDING_TH;
	}
	else if (source.compare("ly") == 0) {
		return ENDING_LY;
	}
	else if (source.compare("ing") == 0) {
		return ENDING_ING;
	}
	else if (source.compare("ings") == 0) {
		return ENDING_INGS;
	}
	else if (source.compare("ed") == 0) {
		return ENDING_ED;
	}
	else if (source.compare("st") == 0) {
		return ENDING_ST;
	}
	else if (source.compare("er") == 0) {
		return ENDING_ER;
	}
	else if (source.compare("ers") == 0) {
		return ENDING_ERS;
	}
	else if (source.compare("ness") == 0) {
		return ENDING_NESS;
	}
	else if (source.compare("able") == 0) {
		return ENDING_ABLE;
	}
	else if (source.compare("ment") == 0) {
		return ENDING_MENT;
	}
	else if (source.compare("ful") == 0) {
		return ENDING_FUL;
	}
	else if (source.compare("ist") == 0) {
		return ENDING_IST;
	}
	else if (source.compare("ists") == 0) {
		return ENDING_ISTS;
	}
	return 0;
}

void addending(tstring &text, int ending){
	int len = text.length();
	if (len <= 2) {
		return;
	}
	switch (ending) {
	case ENDING_S:
		if(text[len - 1] == TEXT('y')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("s");
				break;
			}
			text.pop_back();
			text += TEXT("ies");
			break;
		}
		else if (text[len - 1] == TEXT('s') || text[len - 1] == TEXT('x') || text[len - 1] == TEXT('z') || text[len - 1] == TEXT('h')) {
			text += TEXT("es");
			break;
		}
		text += TEXT("s");
		break;
	case ENDING_IVE:
		if (text[len - 1] == TEXT('e'))
			text.pop_back();
		text += TEXT("ive");
		break;
	case ENDING_ION:
		if (text[len - 1] == TEXT('y')) {
			text.pop_back();
			text += TEXT("ication");
			break;
		}
		else if (text[len - 1] == TEXT('e')) {
			text.pop_back();
			text += TEXT("ion");
			break;
		}
		text += TEXT("en");
		break;
	case ENDING_IONS:
		if (text[len - 1] == TEXT('y')) {
			text.pop_back();
			text += TEXT("ications");
			break;
		}
		else if (text[len - 1] == TEXT('e')) {
			text.pop_back();
			text += TEXT("ions");
			break;
		}
		text += TEXT("ens");
		break;
	case ENDING_TH:
		if (text[len - 1] == TEXT('y')) {
			text.pop_back();
			text += TEXT("ieth");
			break;
		}
		text += TEXT("th");
		break;
	case ENDING_LY:
		if (text[len - 1] == TEXT('c')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("ally");
				break;
			}
		}
		text += TEXT("ly");
		break;
	case ENDING_ING:
		if (text[len - 1] == TEXT('e'))
			text.pop_back();
		text += TEXT("ing");
		break;
	case ENDING_INGS:
		if (text[len - 1] == TEXT('e'))
			text.pop_back();
		text += TEXT("ings");
		break;
	case ENDING_ED:
		if (text[len - 1] == TEXT('e')) {
			text += TEXT("d");
			break;
		}
		if (text[len - 1] == TEXT('y')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("ed");
				break;
			}
			text.pop_back();
			text += TEXT("ied");
			break;
		}
		text += TEXT("ed");
		break;
	case ENDING_ST:
		if (text[len - 1] == TEXT('e')) {
			text += TEXT("st");
			break;
		}
		if (text[len - 1] == TEXT('y')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("est");
				break;
			}
			text.pop_back();
			text += TEXT("iest");
			break;
		}
		text += TEXT("est");
		break;
	case ENDING_ER:
		if (text[len - 1] == TEXT('e')) {
			text += TEXT("r");
			break;
		}
		if (text[len - 1] == TEXT('y')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("er");
				break;
			}
			text.pop_back();
			text += TEXT("ier");
			break;
		}
		text += TEXT("er");
		break;
	case ENDING_ERS:
		if (text[len - 1] == TEXT('e')) {
			text += TEXT("rs");
			break;
		}
		if (text[len - 1] == TEXT('y')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("ers");
				break;
			}
			text.pop_back();
			text += TEXT("iers");
			break;
		}
		text += TEXT("ers");
		break;
	case ENDING_NESS:
		if (text[len - 1] == TEXT('y')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("ness");
				break;
			}
			text.pop_back();
			text += TEXT("iness");
			break;
		}
		text += TEXT("ness");
		break;
	case ENDING_ABLE:
		if (text[len - 1] == TEXT('e')) {
			if (text[len - 2] == TEXT('e') || text[len - 2] == TEXT('g') || text[len - 2] == TEXT('c')) {
				text += TEXT("able");
				break;
			}
			text.pop_back();
			text += TEXT("able");
			break;
		}
		if (text[len - 1] == TEXT('y')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("able");
				break;
			}
			text.pop_back();
			text += TEXT("able");
			break;
		}
		text += TEXT("able");
		break;
	case ENDING_MENT:
		text += TEXT("ment");
		break;
	case ENDING_FUL:
		if (text[len - 1] == TEXT('y')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("ful");
				break;
			}
			text.pop_back();
			text += TEXT("iful");
			break;
		}
		text += TEXT("ful");
		break;
	case ENDING_IST:
		if (text[len - 1] == TEXT('y')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("ist");
				break;
			}
			text.pop_back();
			text += TEXT("ist");
			break;
		}
		text += TEXT("ist");
		break;
	case ENDING_ISTS:
		if (text[len - 1] == TEXT('y')) {
			if (isvowel(text[len - 2])) {
				text += TEXT("ists");
				break;
			}
			text.pop_back();
			text += TEXT("ists");
			break;
		}
		text += TEXT("ists");
		break;
	}
}