#include <cstdio>
#include <cstdlib>
#include <uWS/uWS.h>

const char* sigStart = "{\"signal\":\"start\"}";
const char* ansPrefix = "{\"answer\":\"";
char countAns1[] = "{\"answer\":0}";
char countAns2[] = "{\"answer\":10}";
char* ansTmp;
char* ans;
int pointer;

int main(int argc, char *argv[]) {
	ansTmp = (char*) malloc(512);
	memcpy(ansTmp, ansPrefix, 11);

	uWS::Hub h;

	h.onConnection([&h](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
		ws->send(sigStart);
	});
	h.onMessage([&h](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
		//printf("<RECV> %s\n", message);

		if (message[16] == 'u') {
			char count = '0';
			int status = 0;

			for (int i = 31; message[i] != '"'; i++) {
				if (message[i] == 'm') {
					status = 1;
				} else if (status == 1 && message[i] == 'o') {
					status = 2;
				} else if (status == 2 && message[i] == 'i') {
					count++;
					status = 0;
				} else {
					status = 0;
				}
			}

			if (count < 58) {
				ans = countAns1;
				ans[10] = count;
				pointer = 12;
			} else {
				ans = countAns2;
				ans[11] = count - 10;
				pointer = 13;
			}
		} else if (message[16] == 'p') {
			ans = message+22;
			memcpy(ans, ansPrefix, 11);

			int status = 0;

			for (pointer = 11; ans[pointer] != '"'; pointer++) {
				if (ans[pointer] == 'm') {
					status = 1;
				} else if (status == 1 && ans[pointer] == 'o') {
					status = 2;
				} else if (status == 2 && ans[pointer] == 'i') {
					status = 0;
					ans[pointer-2] = 'M';
					ans[pointer-1] = 'O';
					ans[pointer] = 'I';
				} else {
					status = 0;
				}
			}

			ans[pointer++] = '"';
			ans[pointer++] = '}';
		} else if (message[16] == 'v') {
			ans = ansTmp;
			int status = 0;

			int i = 33;
			for(; message[i] != '"'; i++);
			i--;

			for (pointer = 11; message[i] != '"'; i--) {
				ans[pointer++] = message[i];

				if (message[i] == 'i') {
					status = 1;
				} else if (status == 1 && message[i] == 'o') {
					status = 2;
				} else if (status == 2 && message[i] == 'm') {
					status = 0;
					ans[pointer-1] = 'i';
					ans[pointer-3] = 'm';
				} else {
					status = 0;
				}
			}

			ans[pointer++] = '"';
			ans[pointer++] = '}';
		} else if (message[16] == 't') {
			ans = ansTmp;
			pointer = 11;
			int status = 0;

			message[26] = message[28] = '_';

			for (int i = 26; message[i] != '"'; i++) {
				if (message[i+3] == 'm' && message[i+4] == 'o' && message[i+5] == 'i') {
					status = 9;
				}
				if (status > 0) {
					if (i >= 29) {
						ans[pointer++] = message[i];
					}
					status--;
				}
			}

			ans[pointer++] = '"';
			ans[pointer++] = '}';
		} else {
			printf("<STOP> %s\n", message);
			h.getDefaultGroup<uWS::CLIENT>().close();
			return;
		}

		ws->send(ans, pointer, uWS::OpCode::TEXT, nullptr, nullptr, false);
		//printf("<SEND> %s\n", ans);
	});

	h.connect("wss://saiyo2020.moi.st/websocket/ae5108bdb3dd4326a6a41693a4d7b0de", nullptr);
	h.run();

	return 0;
}
