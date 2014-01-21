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

void addending(std::string &text, int ending){
	int len = text.length();
	if (len <= 2) {
		return;
	}
	switch (ending) {
	case ENDING_S:
		if(text[len - 1] == 'y') {
			if (isvowel(text[len - 2])) {
				text += "s";
				break;
			}
			text.pop_back();
			text += "ies";
			break;
		}
		else if (text[len - 1] == 's' || text[len - 1] == 'x' || text[len - 1] == 'z' || text[len - 1] == 'h') {
			text += "es";
			break;
		}
		text += "s";
		break;
	case ENDING_IVE:
		if (text[len - 1] == 'e')
			text.pop_back();
		text += "ive";
		break;
	case ENDING_ION:
		if (text[len - 1] == 'y') {
			text.pop_back();
			text += "ication";
			break;
		}
		else if (text[len - 1] == 'e') {
			text.pop_back();
			text += "ion";
			break;
		}
		text += "en";
		break;
	case ENDING_IONS:
		if (text[len - 1] == 'y') {
			text.pop_back();
			text += "ications";
			break;
		}
		else if (text[len - 1] == 'e') {
			text.pop_back();
			text += "ions";
			break;
		}
		text += "ens";
		break;
	case ENDING_TH:
		if (text[len - 1] == 'y') {
			text.pop_back();
			text += "ieth";
			break;
		}
		text += "th";
		break;
	case ENDING_LY:
		if (text[len - 1] == 'c') {
			if (isvowel(text[len - 2])) {
				text += "ally";
				break;
			}
		}
		text += "ly";
		break;
	case ENDING_ING:
		if (text[len - 1] == 'e') 
			text.pop_back();
		text += "ing";
		break;
	case ENDING_INGS:
		if (text[len - 1] == 'e') 
			text.pop_back();
		text += "ings";
		break;
	case ENDING_ED:
		if (text[len - 1] == 'e') {
			text += "d";
			break;
		}
		if (text[len - 1] == 'y') {
			if (isvowel(text[len - 2])) {
				text += "ed";
				break;
			}
			text.pop_back();
			text += "ied";
			break;
		}
		text += "ed";
		break;
	case ENDING_ST:
		if (text[len - 1] == 'e') {
			text += "st";
			break;
		}
		if (text[len - 1] == 'y') {
			if (isvowel(text[len - 2])) {
				text += "est";
				break;
			}
			text.pop_back();
			text += "iest";
			break;
		}
		text += "est";
		break;
	case ENDING_ER:
		if (text[len - 1] == 'e') {
			text += "r";
			break;
		}
		if (text[len - 1] == 'y') {
			if (isvowel(text[len - 2])) {
				text += "er";
				break;
			}
			text.pop_back();
			text += "ier";
			break;
		}
		text += "er";
		break;
	case ENDING_ERS:
		if (text[len - 1] == 'e') {
			text += "rs";
			break;
		}
		if (text[len - 1] == 'y') {
			if (isvowel(text[len - 2])) {
				text += "ers";
				break;
			}
			text.pop_back();
			text += "iers";
			break;
		}
		text += "ers";
		break;
	case ENDING_NESS:
		if (text[len - 1] == 'y') {
			if (isvowel(text[len - 2])) {
				text += "ness";
				break;
			}
			text.pop_back();
			text += "iness";
			break;
		}
		text += "ness";
		break;
	case ENDING_ABLE:
		if (text[len - 1] == 'e') {
			if (text[len - 2] == 'e' || text[len - 2] == 'g' || text[len - 2] == 'c') {
				text += "able";
				break;
			}
			text.pop_back();
			text += "able";
			break;
		}
		if (text[len - 1] == 'y') {
			if (isvowel(text[len - 2])) {
				text += "able";
				break;
			}
			text.pop_back();
			text += "able";
			break;
		}
		text += "able";
		break;
	case ENDING_MENT:
		text += "ment";
		break;
	case ENDING_FUL:
		if (text[len - 1] == 'y') {
			if (isvowel(text[len - 2])) {
				text += "ful";
				break;
			}
			text.pop_back();
			text += "iful";
			break;
		}
		text += "ful";
		break;
	case ENDING_IST:
		if (text[len - 1] == 'y') {
			if (isvowel(text[len - 2])) {
				text += "ist";
				break;
			}
			text.pop_back();
			text += "ist";
			break;
		}
		text += "ist";
		break;
	case ENDING_ISTS:
		if (text[len - 1] == 'y') {
			if (isvowel(text[len - 2])) {
				text += "ists";
				break;
			}
			text.pop_back();
			text += "ists";
			break;
		}
		text += "ists";
		break;
	}
}