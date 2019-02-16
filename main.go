package main

import (
	"golang.org/x/net/websocket"
)

func main() {
	var ans []byte
	buffer := make([]byte, 2048)
	sigStart := []byte("{\"signal\":\"start\"}")
	ansPrefix := []byte("{\"answer\":\"")
	ansSuffix := []byte("\"}")

	ws, err := websocket.Dial("wss://saiyo2020.moi.st/websocket/ae5108bdb3dd4326a6a41693a4d7b0de", "", "https://saiyo2020.moi.st/")
	if err != nil {
		panic(err)
	}

	_, err = ws.Write(sigStart)
	if err != nil {
		panic(err)
	}

	for {
		_, err = ws.Read(buffer)
		if err != nil {
			panic(err)
		}
		//fmt.Println("<RECV>", string(buffer))

		if buffer[16] == 117 {
			var count byte
			count = 48
			status := 0

			for i := 31; buffer[i] != 34; i++ {
				if buffer[i] == 109 {
					status = 1
				} else if status == 1 && buffer[i] == 111 {
					status = 2
				} else if status == 2 && buffer[i] == 105 {
					count++
					status = 0
				} else {
					status = 0
				}
			}

			ans = append(make([]byte, 0, 64), ansPrefix...)
			ans = append(ans, count)
			ans = append(ans, ansSuffix...)
		} else if buffer[16] == 112 {
			text := make([]byte, 256)
			status := 0

			i := 33
			for ; buffer[i] != 34; i++ {
				text[i-33] = buffer[i]
				if buffer[i] == 109 {
					status = 1
				} else if status == 1 && buffer[i] == 111 {
					status = 2
				} else if status == 2 && buffer[i] == 105 {
					status = 0
					text[i-35] = 77
					text[i-34] = 79
					text[i-33] = 73
				} else {
					status = 0
				}
			}

			ans = append(make([]byte, 0, 64), ansPrefix...)
			ans = append(ans, text[:i-33]...)
			ans = append(ans, ansSuffix...)
		} else if buffer[16] == 118 {
			text := make([]byte, 256)
			status := 0

			i := 33
			for ; buffer[i] != 34; i++ {
				text[255-(i-33)] = buffer[i]
				if buffer[i] == 109 {
					status = 1
				} else if status == 1 && buffer[i] == 111 {
					status = 2
				} else if status == 2 && buffer[i] == 105 {
					status = 0
					text[255-(i-33)] = 109
					text[255-(i-35)] = 105
				} else {
					status = 0
				}
			}

			ans = append(make([]byte, 0, 64), ansPrefix...)
			ans = append(ans, text[256-(i-33):]...)
			ans = append(ans, ansSuffix...)
		} else if buffer[16] == 116 {
			text := make([]byte, 256)
			pointer := 0
			counter := 0

			buffer[26] = 0
			buffer[28] = 0

			i := 26
			for ; buffer[i] != 34; i++ {
				if buffer[i+3] == 109 && buffer[i+4] == 111 && buffer[i+5] == 105 {
					counter = 9
				}
				if counter > 0 {
					if i >= 29 {
						text[pointer] = buffer[i]
						pointer++
					}
					counter--
				}
			}

			ans = append(make([]byte, 0, 64), ansPrefix...)
			ans = append(ans, text[:pointer]...)
			ans = append(ans, ansSuffix...)
		} else {
			panic(string(buffer))
		}

		//fmt.Println("<SEND>", string(ans))
		_, err = ws.Write(ans)
		if err != nil {
			panic(err)
		}
	}
}
