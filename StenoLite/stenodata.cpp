#include "stdafx.h"
#include "stenodata.h"
#include <string>
#include "texthelpers.h"
#include <cctype>

int countStrokes(const TCHAR* text, const int &len) {
	int total = 1;
	for (int i = 0; i < len; i++) {
		if (text[i] == 0)
			break;
		if (text[i] == '/')
			total++;
	}
	return total;
}

int countStrokes(const std::string &text, const int &len) {
	int total = 1;
	for (std::string::const_iterator i = text.cbegin(); i != text.cend(); i++) {
		if (*i == '/')
			total++;
	}
	return total;
}

int getsecondary(DB *secondary, const DBT *pkey, const DBT *pdata, DBT *skey)

{
	memset(skey, 0, sizeof(DBT));
	skey->data = pdata->data;
	skey->size = pdata->size;
	return (0);
}

// 0123456789012345678901
//#STKPWHRAO*EUFRPBLGTSDZ

inline void searchposition(unsigned __int8* dest, int &position, const char &in, const std::string &format) {
	if (std::toupper(format[position+1]) == std::toupper(in)) {
		if (position < 8) {
			dest[0] |= 1 << position;
		}
		else if (position < 16) {
			dest[1] |= 1 << (position - 8);
		}
		else if (position < 24) {
			dest[2] |= 1 << (position - 16);
		}
		return;
	}

	int stop = position;
	position++;

	while (position != stop) {

		if (std::toupper(format[position+1]) == std::toupper(in)) {
			if (position < 8) {
				dest[0] |= 1 << position;
			}
			else if (position < 16) {
				dest[1] |= 1 << (position - 8);
			}
			else if (position < 24) {
				dest[2] |= 1 << (position - 16);
			}
			return;
		}

		position++;
		if (position >= 22)
			position = 0;
	}
}

void textToStroke(unsigned __int8* dest, std::string::const_iterator &i, std::string::const_iterator &end, const std::string &format) {
	dest[0] = dest[1] = dest[2]  = 0;
	int position = 0;
	for (; i != end; i++) {
		switch (*i) {
		case '/':
			i++;
			return;
		case '-':
			position = 12;
			break;
		case '1':
			dest[0] |= 0x01;
			dest[2] |= 0x40;
			position = 0;
			break;
		case '2':
			dest[0] |= 0x02;
			dest[2] |= 0x40;
			position = 1;
			break;
		case '3':
			dest[0] |= 0x08;
			dest[2] |= 0x40;
			position = 3;
			break;
		case '4':
			dest[0] |= 0x20;
			dest[2] |= 0x40;
			position = 5;
			break;
		case '5':
			dest[0] |= 0x80;
			dest[2] |= 0x40;
			position = 7;
			break;
		case '0':
			dest[1] |= 0x01;
			dest[2] |= 0x40;
			position = 8;
			break;
		case '6':
			dest[1] |= 0x10;
			dest[2] |= 0x40;
			position = 12;
			break;
		case '7':
			dest[1] |= 0x40;
			dest[2] |= 0x40;
			position = 14;
			break;
		case '8':
			dest[2] |= 0x01;
			dest[2] |= 0x40;
			position = 16;
			break;
		case '9':
			dest[2] |= 0x04;
			dest[2] |= 0x40;
			position = 18;
			break;
		default:
			if (*i == format[0]) {
				dest[2] |= 0x40;
				position = 0;
				break;
			}
			searchposition(dest, position, *i, format);
		}
	}
}

/*void textToStroke(unsigned __int8* dest, std::string::const_iterator &i, std::string::const_iterator &end) {
	dest[0] = dest[1] = dest[2] = 0;
	bool lead = true;
	for (; i != end; i++) {
		switch (*i) {
		case '/':
			i++;
			return;
		case 's':
		case 'S':
			if (lead) {
				dest[0] |= 0x01;
			}
			else {
				dest[2] |= 0x08;
			}
			break;
		case 't':
		case 'T':

			if (lead) {
				dest[0] |= 0x02;
			}
			else {
				dest[2] |= 0x04;
			}
			break;
		case 'k':
		case 'K':
			dest[0] |= 0x04;
			break;
		case 'p':
		case 'P':
			if (lead) {
				dest[0] |= 0x08;
			}
			else {
				dest[1] |= 0x40;
			}
			break;
		case 'w':
		case 'W':
			dest[0] |= 0x10;
			break;
		case 'h':
		case 'H':
			dest[0] |= 0x20;
			break;
		case 'r':
		case 'R':
			if (lead) {
				dest[0] |= 0x40;
			}
			else {
				dest[1] |= 0x20;
			}
			break;
		case 'a':
		case 'A':
			dest[0] |= 0x80;
			lead = false;
			break;
		case 'o':
		case 'O':
			dest[1] |= 0x01;
			lead = false;
			break;
		case '*':
			dest[1] |= 0x02;
			lead = false;
			break;
		case '-':
			lead = false;
			break;
		case 'e':
		case 'E':
			dest[1] |= 0x04;
			lead = false;
			break;
		case 'u':
		case 'U':
			dest[1] |= 0x08;
			lead = false;
			break;
		case 'f':
		case 'F':
			dest[1] |= 0x10;
			lead = false;
			break;
		case 'b':
		case 'B':
			dest[1] |= 0x80;
			lead = false;
			break;
		case 'l':
		case 'L':
			dest[2] |= 0x01;
			lead = false;
			break;
		case 'g':
		case 'G':
			dest[2] |= 0x02;
			lead = false;
			break;
		case 'd':
		case 'D':
			dest[2] |= 0x10;
			lead = false;
			break;
		case 'z':
		case 'Z':
			dest[2] |= 0x20;
			lead = false;
			break;
		case '#':
			dest[2] |= 0x40;
			break;
		case '1':
			dest[0] |= 0x01;
			dest[2] |= 0x40;
			break;
		case '2':
			dest[0] |= 0x02;
			dest[2] |= 0x40;
			break;
		case '3':
			dest[0] |= 0x08;
			dest[2] |= 0x40;
			break;
		case '4':
			dest[0] |= 0x20;
			dest[2] |= 0x40;
			break;
		case '5':
			dest[0] |= 0x80;
			dest[2] |= 0x40;
			lead = false;
			break;
		case '0':
			dest[1] |= 0x01;
			dest[2] |= 0x40;
			lead = false;
			break;
		case '6':
			dest[1] |= 0x10;
			dest[2] |= 0x40;
			lead = false;
			break;
		case '7':
			dest[1] |= 0x40;
			dest[2] |= 0x40;
			lead = false;
			break;
		case '8':
			dest[2] |= 0x01;
			dest[2] |= 0x40;
			lead = false;
			break;
		case '9':
			dest[2] |= 0x04;
			dest[2] |= 0x40;
			lead = false;
			break;
		}
	}
}*/

void textToStroke(const std::string &stro, unsigned __int8* dest, const std::string &format) {
	textToStroke(dest, stro.cbegin(), stro.cend(), format);
}

unsigned __int8* texttomultistroke(const std::string &in, int& size, const std::string &format) {
	size = countStrokes(in, size);
	unsigned __int8* sdata = new unsigned __int8[size * 3];

	auto it = in.cbegin();
	int i = 0;
	bool empty = true;
	while (it != in.cend()) {
		textToStroke(sdata + i * 3, it, in.cend(), format);
		i++;
		empty = false;
	}
	if (empty)
		size = 0;
	return sdata;
}

void dictionary::addNewDItem(unsigned __int8 *s, const int &len, const std::string &str, DB_TXN* trans) {


	if (str.length() > lchars) {
		lchars = str.length();
	}
	if (len / 3 > longest) {
		longest = len / 3;
	}
	DBT keyin;
	memset(&keyin, 0, sizeof(DBT));
	keyin.data = s;
	keyin.size = len;
	keyin.ulen = len;
	keyin.flags = DB_DBT_USERMEM;

	DBT strin;
	memset(&strin, 0, sizeof(DBT));
	strin.data = (void*)(str.c_str());
	strin.size = str.length() + 1;
	strin.ulen = str.length() + 1;
	strin.flags = DB_DBT_USERMEM;

	if (contents->put(contents, trans, &keyin, &strin, DB_NOOVERWRITE) == 0) {
		items++;
	}
}

void dictionary::addDItem(unsigned __int8 *s, const int &len, const std::string &str, DB_TXN* trans) {
	
	if (str.length() > lchars) {
		lchars = str.length();
	}
	if (len / 3 > longest) {
		longest = len / 3;
	}
	DBT keyin;
	memset(&keyin, 0, sizeof(DBT));
	keyin.data = s;
	keyin.size = len;
	keyin.ulen = len;
	keyin.flags = DB_DBT_USERMEM;

	
	DBT strin;
	memset(&strin, 0, sizeof(DBT));
	strin.data = (void*)(str.c_str());
	strin.size = str.length() + 1;
	strin.ulen = str.length() + 1;
	strin.flags = DB_DBT_USERMEM;

	if (contents->exists(contents, trans, &keyin, DB_RMW) == DB_NOTFOUND) {
		items++;
	}
	contents->put(contents, trans, &keyin, &strin, 0);
}

void dictionary::deleteDItem(unsigned __int8 *s, const int &len, DB_TXN* trans) {
	DBT keyin;
	keyin.data = s;
	keyin.size = len;
	keyin.ulen = len;
	keyin.dlen = 0;
	keyin.doff = 0;
	keyin.flags = DB_DBT_USERMEM;
	if (contents->del(contents, trans, &keyin, 0) == 0)
		items--;
}

bool dictionary::findDItem(unsigned __int8 *s, const int &len, std::string &str, DB_TXN* trans) {

	DBT keyin;
	memset(&keyin, 0, sizeof(DBT));
	keyin.data = s;
	keyin.size = len;
	keyin.ulen = len;
	keyin.flags = DB_DBT_USERMEM;

	if (contents->exists(contents, trans, &keyin, 0) == DB_NOTFOUND) {
		return false;
	}

	DBT strin;
	memset(&strin, 0, sizeof(DBT));
	strin.data = new unsigned __int8[lchars + 1];
	strin.ulen = lchars + 1;
	strin.flags = DB_DBT_USERMEM;


	if (contents->get(contents, trans, &keyin, &strin, 0) == 0) {
		str = std::string((char*)(strin.data));
		delete strin.data;
		return true;
	}

	delete strin.data;
	return false;
}

void stroketocsteno(unsigned __int8* keys, std::string &buffer, const std::string &format, bool number) {
	static TCHAR internal[200];
	//buffer.clear();
	
	stroketocsteno(keys, internal, format, number);
	for (int i = 0; internal[i] != 0; i++) {
		buffer += (char)(internal[i]);
	}
}

void stroketocsteno(unsigned __int8* keys, std::wstring &buffer, const std::string &format, bool number) {
	static TCHAR internal[200];
	//buffer.clear();
	stroketocsteno(keys, internal, format, number);
	for (int i = 0; internal[i] != 0; i++) {
		buffer += internal[i];
	}
}

void stroketocsteno(unsigned __int8* keys, TCHAR* buffer, const std::string &format, bool number) {
	int index = 0;

	bool numbers = false;

	if ((keys[2] & 0x40) != 0) {
		numbers = true;
		if ((keys[0] & (0x01 | 0x02 | 0x08 | 0x20 | 0x80)) != 0 || (keys[1] & (0x01 | 0x10 | 0x40)) != 0 || (keys[2] & (0x01 | 0x04)) != 0) {

		}
		else {
			buffer[index] = format[0];
			index++;
		}
	}

	bool separated = false;

	if ((keys[0] & 0x01) != 0) {
		if (numbers) {
			buffer[index] = TEXT('1');
		}
		else {
			buffer[index] = format[1];
		}
		index++;
	}
	if ((keys[0] & 0x02) != 0) {
		if (numbers) {
			buffer[index] = TEXT('2');
		}
		else {
			buffer[index] = format[2];
		}
		index++;
	}
	if ((keys[0] & 0x04) != 0) {
		buffer[index] = format[3];
		index++;
	}
	if ((keys[0] & 0x08) != 0) {
		if (numbers) {
			buffer[index] = TEXT('3');
		}
		else {
			buffer[index] = format[4];
		}
		index++;
	}
	if ((keys[0] & 0x10) != 0) {
		buffer[index] = format[5];
		index++;
	}
	if ((keys[0] & 0x20) != 0) {
		if (numbers) {
			buffer[index] = TEXT('4');
		}
		else {
			buffer[index] = format[6];
		}
		index++;
	}
	if ((keys[0] & 0x40) != 0) {
		buffer[index] = format[7];
		index++;
	}
	if ((keys[0] & 0x80) != 0) {
		if (numbers) {
			buffer[index] = TEXT('5');
		}
		else {
			buffer[index] = format[8];
		}
		index++;
		separated = true;
	}
	if ((keys[1] & 0x01) != 0) {
		if (numbers) {
			buffer[index] = TEXT('0');
		}
		else {
			buffer[index] = format[9];
		}
		index++;
		separated = true;
	}
	if ((keys[1] & 0x02) != 0) {
		buffer[index] = format[10];
		index++;
		separated = true;
	}
	if ((keys[1] & 0x04) != 0) {
		buffer[index] = format[11];
		index++;
		separated = true;
	}
	if ((keys[1] & 0x08) != 0) {
		buffer[index] = format[12];
		index++;
		separated = true;
	}
	if (!separated && !number) {
		buffer[index] = TEXT('-');
		index++;
	}
	if ((keys[1] & 0x10) != 0) {
		if (numbers) {
			buffer[index] = TEXT('6');
		}
		else {
			buffer[index] = format[13];
		}
		index++;
	}
	if ((keys[1] & 0x20) != 0) {
		buffer[index] = format[14];
		index++;
	}
	if ((keys[1] & 0x40) != 0) {
		if (numbers) {
			buffer[index] = TEXT('7');
		}
		else {
			buffer[index] = format[15];
		}
		index++;
	}
	if ((keys[1] & 0x80) != 0) {
		buffer[index] = format[16];
		index++;
	}
	if ((keys[2] & 0x01) != 0) {
		if (numbers) {
			buffer[index] = TEXT('8');
		}
		else {
			buffer[index] = format[17];
		}
		index++;
	}
	if ((keys[2] & 0x02) != 0) {
		buffer[index] = format[18];
		index++;
	}
	if ((keys[2] & 0x04) != 0) {
		if (numbers) {
			buffer[index] = TEXT('9');
		}
		else {
			buffer[index] = format[19];
		}
		index++;
	}
	if ((keys[2] & 0x08) != 0) {
		buffer[index] = format[20];
		index++;
	}
	if ((keys[2] & 0x10) != 0) {
		buffer[index] = format[21];
		index++;
	}
	if ((keys[2] & 0x20) != 0) {
		buffer[index] = format[22];
		index++;
	}

	buffer[index] = 0;

}

void stroketosteno(unsigned __int8* keys, TCHAR* buffer, const std::string &format) {
	_tcscpy_s(buffer, 24, TEXT("                       "));
	bool numbers = false;
	bool wrotenumbers = false;

	if ((keys[2] & 0x40) != 0) {
		numbers = true;
	}

	if ((keys[0] & 0x01) != 0) {
		if (numbers) {
			buffer[1] = TEXT('1');
			wrotenumbers = true;
		}
		else {
			buffer[1] = format[1];
		}
	}
	if ((keys[0] & 0x02) != 0) {
		if (numbers) {
			buffer[2] = TEXT('2');
			wrotenumbers = true;
		}
		else {
			buffer[2] = format[2];
		}
	}
	if ((keys[0] & 0x04) != 0) {
		buffer[3] = format[3];
	}
	if ((keys[0] & 0x08) != 0) {
		if (numbers) {
			buffer[4] = TEXT('3');
			wrotenumbers = true;
		}
		else {
			buffer[4] = format[4];
		}
	}
	if ((keys[0] & 0x10) != 0) {
		buffer[5] = format[5];
	}
	if ((keys[0] & 0x20) != 0) {
		if (numbers) {
			buffer[6] = TEXT('4');
			wrotenumbers = true;
		}
		else {
			buffer[6] = format[6];
		}
	}
	if ((keys[0] & 0x40) != 0) {
		buffer[7] = format[7];
	}
	if ((keys[0] & 0x80) != 0) {
		if (numbers) {
			buffer[8] = TEXT('5');
			wrotenumbers = true;
		}
		else {
			buffer[8] = format[8];
		}
	}
	if ((keys[1] & 0x01) != 0) {
		if (numbers) {
			buffer[9] = TEXT('0');
			wrotenumbers = true;
		}
		else {
			buffer[9] = format[9];
		}
	}
	if ((keys[1] & 0x02) != 0) {
		buffer[10] = format[10];
	}
	if ((keys[1] & 0x04) != 0) {
		buffer[11] = format[11];
	}
	if ((keys[1] & 0x08) != 0) {
		buffer[12] = format[12];
	}
	if ((keys[1] & 0x10) != 0) {
		if (numbers) {
			buffer[13] = TEXT('6');
			wrotenumbers = true;
		}
		else {
			buffer[13] = format[13];
		}
	}
	if ((keys[1] & 0x20) != 0) {
		buffer[14] = format[14];
	}
	if ((keys[1] & 0x40) != 0) {
		if (numbers) {
			buffer[15] = TEXT('7');
			wrotenumbers = true;
		}
		else {
			buffer[15] = format[15];
		}
	}
	if ((keys[1] & 0x80) != 0) {
		buffer[16] = format[16];
	}
	if ((keys[2] & 0x01) != 0) {
		if (numbers) {
			buffer[17] = TEXT('8');
			wrotenumbers = true;
		}
		else {
			buffer[17] = format[17];
		}
	}
	if ((keys[2] & 0x02) != 0) {
		buffer[18] = format[18];
	}
	if ((keys[2] & 0x04) != 0) {
		if (numbers) {
			buffer[19] = TEXT('9');
			wrotenumbers = true;
		}
		else {
			buffer[19] = format[19];
		}
	}
	if ((keys[2] & 0x08) != 0) {
		buffer[20] = format[20];
	}
	if ((keys[2] & 0x10) != 0) {
		buffer[21] = format[21];
	}
	if ((keys[2] & 0x20) != 0) {
		buffer[22] = format[22];
	}

	if (numbers && (wrotenumbers == false)) {
		buffer[0] = format[0];
	}
}

bool dictionary::opencrecovery(const char* file, const char* file2) {
	

		db_env_create(&env, 0);
		env->set_lk_detect(env, DB_LOCK_MINWRITE);
		env->log_set_config(env, DB_LOG_AUTO_REMOVE, 1);
		env->set_lg_max(env, 1048576);
		env->open(env, hm.c_str(), DB_INIT_MPOOL | DB_CREATE | DB_INIT_MPOOL | DB_INIT_TXN | DB_REGISTER | DB_RECOVER_FATAL | DB_INIT_LOCK | DB_INIT_LOG | DB_THREAD, 0);

		if (contents->open(contents, NULL, file, NULL, DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT | DB_READ_UNCOMMITTED, 0) != 0) {
			return false;
		}
	



	db_create(&secondary, env, 0);
	secondary->set_flags(secondary, DB_DUP | DB_DUPSORT);

	if ((secondary->open(secondary, NULL, file2, NULL, DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT | DB_READ_UNCOMMITTED, 0)) != 0) {
		//MessageBox(NULL, TEXT("Failed to open secondary index"), TEXT("Error"), MB_OK);
	}
	if ((contents->associate(contents, NULL, secondary, getsecondary, DB_AUTO_COMMIT)) != 0) {
		//MessageBox(NULL, TEXT("Failed to associate index"), TEXT("Error"), MB_OK);
	}



	return true;
}

void errcall(const DB_ENV* env, const char *a, const char*b){
	OutputDebugStringA(a);
	OutputDebugStringA("\r\n");
	OutputDebugStringA(b);
	OutputDebugStringA("\r\n");
}


bool dictionary::open(const char* file, const char* file2) {

	db_env_create(&env, 0);
	//env->set_errcall(env, &errcall);

	env->set_lk_detect(env, DB_LOCK_MINWRITE);
	env->log_set_config(env, DB_LOG_AUTO_REMOVE, 1);
	env->set_lg_max(env, 1048576);

	env->open(env, hm.c_str(), DB_INIT_MPOOL | DB_CREATE | DB_INIT_MPOOL | DB_REGISTER | DB_INIT_TXN | DB_RECOVER | DB_INIT_LOCK | DB_INIT_LOG | DB_THREAD, 0);


	db_create(&contents, env, 0);
	
	
	if (contents->open(contents, NULL, file, NULL, DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT | DB_READ_UNCOMMITTED, 0) != 0) {
		return false;

	}
	
	db_create(&secondary, env, 0);
	secondary->set_flags(secondary, DB_DUP | DB_DUPSORT);

	if ((secondary->open(secondary, NULL, file2, NULL, DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT | DB_READ_UNCOMMITTED, 0)) != 0) {
		//MessageBox(NULL, TEXT("Failed to open secondary index"), TEXT("Error"), MB_OK);
	}
	if ((contents->associate(contents, NULL, secondary, getsecondary, DB_AUTO_COMMIT)) != 0) {
		//MessageBox(NULL, TEXT("Failed to associate index"), TEXT("Error"), MB_OK);
	}
	


	return true;


}



dictionary::dictionary(const char *home) {
	hm = home;
	
	
}

void dictionary::close() {
	env->txn_checkpoint(env, 0, 0, DB_FORCE);
	secondary->close(secondary, 0);
	contents->close(contents, 0);
	env->close(env, 0);
}