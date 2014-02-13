#include "stdafx.h"
#include "broadcast.h"

#include <boost/network/include/http/server.hpp>
#include <boost/network/uri.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include <exception>
#include <list>
#include "texthelpers.h"
#include "globals.h"


const char pageptA[] = "<html>\n\
<body>\n\
<div id = \"maintext\" style = \"overflow:auto; width:98%; height:98%; border:1px solid black; \">";

//existing text between these

const char pageptB[] = "</div>\n\
<script>\n\
var t = ";

//initial request number between these

const char pageptC[] = ";\n\
var xmlhttp;\n\
\n\
if (window.XMLHttpRequest) {\n\
	xmlhttp = new XMLHttpRequest();\n\
}\n\
else {\n\
	xmlhttp = new ActiveXObject(\"Microsoft.XMLHTTP\");\n\
}\n\
\n\
function ProcessResponse() {\n\
	if (xmlhttp.readyState == 4  && xmlhttp.status==200 && xmlhttp.responseText.length > 0) {\n\
		if (xmlhttp.responseText.charAt(0) === \"Q\") {\n\
		} else if (xmlhttp.responseText.charAt(0) === \"X\") {\n\
			var indx = 2;\n\
			var newt = \"\";\n\
			var atbottom = (document.getElementById(\"maintext\").scrollHeight - document.getElementById(\"maintext\").scrollTop === document.getElementById(\"maintext\").clientHeight);\n\
			while (indx < xmlhttp.responseText.length && xmlhttp.responseText.charAt(indx) != ':') {\n\
				newt += xmlhttp.responseText.charAt(indx);\n\
				indx = indx + 1;\n\
			}\n\
			if(newt.length > 0){\n\
				t = Number(newt);\n\
				document.documentElement.appendChild(document.createTextNode(t));\n\
			} else {\n\
				t = 0\n\
				document.documentElement.appendChild(document.createTextNode(t));\n\
			}\n\
			indx = indx + 1;\n\
			document.getElementById(\"maintext\").innerHTML = xmlhttp.responseText.substr(indx, xmlhttp.responseText.length - indx);\n\
			if (atbottom) {\n\
				document.getElementById(\"maintext\").scrollTop = document.getElementById(\"maintext\").clientHeight;\n\
			}\n\
			window.setTimeout(function(){var req = \"/update.html?t=\" + t;\n\
				xmlhttp.open(\"GET\", req, true);\n\
				xmlhttp.onreadystatechange = ProcessResponse;\n\
				xmlhttp.send();}, 1000);\n\
		}\n\
		else {\n\
			var indx = 0;\n\
			var dels = \"\";\n\
			var newt = \"\";\n\
			var atbottom = (document.getElementById(\"maintext\").scrollHeight - document.getElementById(\"maintext\").scrollTop === document.getElementById(\"maintext\").clientHeight);\n\
			while (indx < xmlhttp.responseText.length && xmlhttp.responseText.charAt(indx) != ':') {\n\
				newt += xmlhttp.responseText.charAt(indx);\n\
				indx = indx + 1;\n\
			}\n\
			if(newt.length > 0){ \
				t = Number(newt);\n\
			} else {\n\
				t = 0\n\
			}\n\
			indx = indx + 1;\n\
			while (indx < xmlhttp.responseText.length && xmlhttp.responseText.charAt(indx) != ':') {\n\
				dels += xmlhttp.responseText.charAt(indx);\n\
				indx = indx + 1;\n\
			}\n\
			indx = indx + 1;\n\
			var curtext = document.getElementById(\"maintext\").innerHTML;\n\
			if(dels.length > 0){ \
				numdel = Number(dels);\n\
			} else {\n\
				numdel = 0\n\
			}\n\
			var deleted = 0;\n\
			var backpos = curtext.length - 1;\n\
			while (deleted < numdel && backpos >= 0) {\n\
				if (curtext.charAt(backpos) == ';') {\n\
					while (curtext.charAt(backpos) != '&' && backpos > 1) {\n\
						backpos -= 1;\n\
					}\n\
					backpos -= 1;\n\
				}\n\
				else if (curtext.charAt(backpos) == '>') {\n\
					while (curtext.charAt(backpos) != '<' && backpos > 1) {\n\
						backpos -= 1;\n\
					}\n\
					backpos -= 1;\n\
				}\n\
				else {\n\
					backpos -= 1;\n\
				}\n\
				deleted += 1;\n\
			}\n\
			curtext = curtext.substr(0, backpos + 1);\n\
			if (indx < xmlhttp.responseText.length) {\n\
				curtext += xmlhttp.responseText.substr(indx, xmlhttp.responseText.length - indx);\n\
			}\n\
			document.getElementById(\"maintext\").innerHTML = curtext;\n\
			if (atbottom) {\n\
				document.getElementById(\"maintext\").scrollTop = document.getElementById(\"maintext\").clientHeight;\n\
			}\n\
			window.setTimeout(function(){var req = \"/update.html?t=\" + t;\n\
				xmlhttp.open(\"GET\", req, true);\n\
				xmlhttp.onreadystatechange = ProcessResponse;\n\
				xmlhttp.send();}, 1000);\n\
		}\n\
	}\n\
}\n\
\n\
window.setTimeout(function(){var req = \"/update.html?t=\" + t;\n\
				xmlhttp.open(\"GET\", req, true);\n\
				xmlhttp.onreadystatechange = ProcessResponse;\n\
				xmlhttp.send();}, 1000);\n\
</script>\n\
</body>\n\
</html>";

HANDLE writemutex;
HANDLE finishedread;
HANDLE closeevent;
HANDLE newmutex;
HANDLE addednewevent;
int reading = 0;
bool serverrunning = false;
bool stop = false;

struct stenoevent {
	unsigned int number;
	unsigned int deletions;
	std::string text;
};

std::list<stenoevent> events;
unsigned int maxpackage = 1;
std::list<stenoevent> newevent;
std::string htmltext;

struct handler;
typedef boost::network::http::async_server<handler> server;

struct handler
{
	void operator()(server::request const& req, const server::connection_ptr& conn)
	{
		WaitForSingleObject(writemutex, INFINITE);
		reading++;
		ReleaseMutex(writemutex);

		static server::response_header headers[] = {
			{ "Connection", "close" }
			, { "Content-Type", "text/html" }
		};

		//
		std::string dest = req.destination;
		int p = dest.find("?t=");
		if (p != std::string::npos) {
			std::string value = dest.substr(p + 3);
			unsigned int val = atoi(value.c_str());
			//send new chunks
			OutputDebugStringA((dest + "\r\n").c_str());

			if (stop) {
				conn->set_status(server::connection::forbidden);
				conn->set_headers(boost::make_iterator_range(headers, headers + 2));
				conn->write("Q::");
				OutputDebugStringA("Q::\r\n");
			}
			else if (val >= maxpackage) {
				conn->set_status(server::connection::ok);
				conn->set_headers(boost::make_iterator_range(headers, headers + 2));
				conn->write(std::to_string(maxpackage) + "::");
				OutputDebugStringA((std::to_string(maxpackage) + "::\r\n").c_str());
			}
			else if (val < events.back().number) {
				conn->set_status(server::connection::ok);
				conn->set_headers(boost::make_iterator_range(headers, headers + 2));
				conn->write("X:" + std::to_string(maxpackage) + ":" + htmltext);
				OutputDebugStringA(("X:" + std::to_string(maxpackage) + ":" + htmltext).c_str());
			}
			else {
				int todelete = 0;
				std::string textosend;
				auto it = events.cend();
				while (it != events.cbegin()) {
					it--;
					if ((*it).number >= val) {
						int d = (*it).deletions;

						while (d > 0) {
							if (textosend.length() > 0) {
								if (textosend.back() == '>') {
									while (textosend.length() > 0 && textosend.back() != '<') {
										textosend.pop_back();
									}
									textosend.pop_back();
								}
								else if (textosend.back() == ';') {
									while (textosend.length() > 0 && textosend.back() != '&') {
										textosend.pop_back();
									}
									textosend.pop_back();
								}
								else {
									textosend.pop_back();
								}
							}
							else {
								todelete++;
							}
							d--;
						}

						textosend += (*it).text;
					}
				}

				//ready to send
				conn->set_status(server::connection::ok);
				conn->set_headers(boost::make_iterator_range(headers, headers + 2));
				conn->write(std::to_string(maxpackage) + ":" + std::to_string(todelete) + ":" + textosend);
				OutputDebugStringA((std::to_string(maxpackage) + ":" + std::to_string(todelete) + ":" + textosend + "\r\n").c_str());
			}

		}
		else {
			//send the whole thing
			conn->set_status(server::connection::ok);
			conn->set_headers(boost::make_iterator_range(headers, headers + 2));
			conn->write(pageptA + htmltext + pageptB + std::to_string(maxpackage) + pageptC);
		}

		WaitForSingleObject(writemutex, INFINITE);
		reading--;
		ReleaseMutex(writemutex);
		SetEvent(finishedread);
	}
};

void AddNewEvent(int numdeletes, const std::string& text) {
	stenoevent s;
	s.deletions = numdeletes;
	s.text = text;
	WaitForSingleObject(newmutex, INFINITE);
	newevent.push_back(s);
	ReleaseMutex(newmutex);
	SetEvent(addednewevent);
}

#define MAXEVENTS 100

DWORD WINAPI ReadInNewEvents(LPVOID lparam) {
	while (serverrunning) {
		WaitForSingleObject(addednewevent, INFINITE);
		if (!serverrunning)
			break;

		WaitForSingleObject(newmutex, INFINITE);
		bool newitems = newevent.size() > 0;
		ReleaseMutex(newmutex);

		if (!newitems)
			continue;
		
		WaitForSingleObject(writemutex, INFINITE);
		while (reading != 0) {
			ReleaseMutex(writemutex);
			WaitForSingleObject(finishedread, INFINITE);
			WaitForSingleObject(writemutex, INFINITE);
		}
		WaitForSingleObject(newmutex, INFINITE);
		// all good
		while (newevent.size() > 0) {
			events.push_front(newevent.front());
			events.front().number = maxpackage++;

			int d = events.front().deletions;
			while (d > 0) {
				if (htmltext.length() > 0) {
					if (htmltext.back() == '>') {
						while (htmltext.length() > 0 && htmltext.back() != '<') {
							htmltext.pop_back();
						}
						htmltext.pop_back();
					}
					else if (htmltext.back() == ';') {
						while (htmltext.length() > 0 && htmltext.back() != '&') {
							htmltext.pop_back();
						}
						htmltext.pop_back();
					}
					else {
						htmltext.pop_back();
					}
				}
				d--;
			}

			std::string temp;

			for (auto ci = events.front().text.cbegin(); ci != events.front().text.cend(); ci++) {
				if (*ci == '\n') {
					temp += "<br>";
				}
				else if (*ci == '<') {
					temp += "&lt;";
				}
				else if (*ci == '>') {
					temp += "&gt;";
				}
				else if (*ci == '&') {
					temp += "&amp;";
				}
				else if (*ci == '\"') {
					temp += "&quot;";
				}
				else if (*ci == ';') {
					temp += "&#59;";
				}
				else {
					temp += *ci;
				}
			}

			htmltext += temp;
			events.front().text = temp;

			newevent.pop_front();
		}

		while (events.size() > MAXEVENTS) {
			events.pop_back();
		}
		
		ReleaseMutex(newmutex);
		ReleaseMutex(writemutex);
	}

	return 0;
}

server* myserver = NULL;

DWORD WINAPI RunServer(LPVOID lparam) {
	int port = (int)lparam;

	finishedread = CreateEvent(NULL, FALSE, FALSE, NULL);
	closeevent = CreateEvent(NULL, FALSE, FALSE, NULL);
	addednewevent = CreateEvent(NULL, FALSE, FALSE, NULL);
	writemutex = CreateMutex(NULL, FALSE, NULL);
	newmutex = CreateMutex(NULL, FALSE, NULL);

	maxpackage = 1;
	reading = 0;
	serverrunning = true;

	CreateThread(NULL, 0, ReadInNewEvents, NULL, 0, NULL);

	handler myhandler;
	server::options options(myhandler);
	options.thread_pool(boost::make_shared<boost::network::utils::thread_pool>(2));
	try {
		myserver = new server(options.address("0.0.0.0").port(std::to_string(port)));

		boost::thread t1(boost::bind(&server::run, myserver));
		boost::thread t2(boost::bind(&server::run, myserver));
		myserver->listen();
		stop = false;
		myserver->run();
		t1.join();
		t2.join();


		WaitForSingleObject(closeevent, INFINITE);
		stop = true;
		myserver->stop();
	}
	catch (...) {
		MessageBox(NULL, TEXT("Could not bind server to this port"), TEXT("Error"), MB_OK);
	}
	delete myserver;
	myserver = NULL;
	serverrunning = false;
	SetWindowText(controls.bserver, TEXT("Broadcast Text"));
	myserver = NULL;
	SetEvent(addednewevent);
	htmltext.clear();
	events.clear();
	newevent.clear();
	CloseHandle(finishedread);
	CloseHandle(closeevent);
	CloseHandle(writemutex);
	CloseHandle(newmutex);
	CloseHandle(addednewevent);
	return 0;
}

bool ServerRunning() {
	return serverrunning;
}

void CloseServer() {
	if (serverrunning) {
		stop = true;
		if (myserver != NULL)
			myserver->stop();
		SetEvent(closeevent);
	}
}