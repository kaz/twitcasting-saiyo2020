#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>

#define NO_DEBUG

#ifdef DEBUG
#define LOG(io, data, len) printf("\n\n---------- %s ----------\n\n", (io) ? "I N" : "OUT"), write(1, (data), (len))
#define FIN(data, len)
#else
#define LOG(io, data, len)
#define FIN(data, len) printf("\n\n---------- FIN ----------\n\n"), write(1, (data), (len))
#endif

const char* opening_handshake = "\
GET /websocket/ae5108bdb3dd4326a6a41693a4d7b0de HTTP/1.1\n\
Sec-WebSocket-Key: Hello/saiyo2020/moi/st\n\
Sec-WebSocket-Version: 13\n\
Host: saiyo2020.moi.st\n\
Connection: Upgrade\n\
Upgrade: websocket\n\
\n";

const char* signal_start = "\x81\x92\0\0\0\0{\"signal\":\"start\"}";
char answer_template[512] = "\x81\xfe\0.\0\0\0\0{\"answer\":\"";
char answer_template_count_1digit[] = "\x81\x8c\0\0\0\0{\"answer\":0}";
char answer_template_count_2digit[] = "\x81\x8d\0\0\0\0{\"answer\":10}";

int main() {
	int read;
	int buf_len = 512;
	char buf[buf_len];
	char* message;
	char* response;
	uint16_t pointer;
	uint16_t status;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(443);
	inet_aton("54.199.26.149", &addr.sin_addr);
	connect(sockfd, (const struct sockaddr*) &addr, 32);

	SSL_library_init();

	SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());
	SSL* ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sockfd);
	SSL_connect(ssl);

	LOG(0, opening_handshake, strlen(opening_handshake));
	SSL_write(ssl, opening_handshake, strlen(opening_handshake));
	read = SSL_read(ssl, buf, buf_len);
	LOG(1, buf, read);

	LOG(0, signal_start, 24);
	SSL_write(ssl, signal_start, 24);

	while (1) {
		/*
		read = SSL_read(ssl, buf, buf_len);
		for(message = buf; *message != '{'; message++, read--);
		*/

		read = SSL_read(ssl, buf, buf_len) - 4;
		message = buf + 4;
		LOG(1, message, read);

		status = 0;

		if (message[16] == 'u') {
			char count = '0';

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
				response = (char*) answer_template_count_1digit;
				response[16] = count;
				pointer = 18;
			} else {
				response = (char*) answer_template_count_2digit;
				response[17] = count - 10;
				pointer = 19;
			}
		} else if (message[16] == 'p') {
			response = message + 14;
			memcpy(response, answer_template, pointer = 19);

			for (; response[pointer] != '"'; pointer++) {
				if (response[pointer] == 'm') {
					status = 1;
				} else if (status == 1 && response[pointer] == 'o') {
					status = 2;
				} else if (status == 2 && response[pointer] == 'i') {
					status = 0;
					response[pointer-2] = 'M';
					response[pointer-1] = 'O';
					response[pointer] = 'I';
				} else {
					status = 0;
				}
			}

			response[pointer++] = '"';
			response[pointer++] = '}';
			response[3] = pointer - 8;
		} else if (message[16] == 'v') {
			response = answer_template;

			int i = 33;
			for(; message[i] != '"'; i++);
			i--;

			for (pointer = 19; message[i] != '"'; i--) {
				response[pointer++] = message[i];

				if (message[i] == 'i') {
					status = 1;
				} else if (status == 1 && message[i] == 'o') {
					status = 2;
				} else if (status == 2 && message[i] == 'm') {
					status = 0;
					response[pointer-1] = 'i';
					response[pointer-3] = 'm';
				} else {
					status = 0;
				}
			}

			response[pointer++] = '"';
			response[pointer++] = '}';
			response[3] = pointer - 8;
		} else if (message[16] == 't') {
			response = answer_template;
			pointer = 19;

			message[26] = message[28] = '_';

			for (int i = 26; message[i] != '"'; i++) {
				if (message[i+3] == 'm' && message[i+4] == 'o' && message[i+5] == 'i') {
					status = 9;
				}
				if (status > 0) {
					if (i >= 29) {
						response[pointer++] = message[i];
					}
					status--;
				}
			}

			response[pointer++] = '"';
			response[pointer++] = '}';
			response[3] = pointer - 8;
		} else {
			break;
		}

		LOG(0, response, pointer);
		SSL_write(ssl, response, pointer);
	}

	SSL_shutdown(ssl);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	close(sockfd);

	FIN(message, read);

	return 0;
}
