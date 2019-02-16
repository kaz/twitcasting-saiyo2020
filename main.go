package main

import (
	"fmt"

	"golang.org/x/net/websocket"
)

func main() {
	var ans []byte
	buffer := make([]byte, 512)
	sigStart := []byte("{\"signal\":\"start\"}")
	countAns1 := []byte("{\"answer\":0}")
	countAns2 := []byte("{\"answer\":10}")
	ansTemplate := append([]byte("{\"answer\":\""), make([]byte, 512)...)

	ws, err := websocket.Dial("wss://saiyo2020.moi.st/websocket/ae5108bdb3dd4326a6a41693a4d7b0de", "", "https://saiyo2020.moi.st/")
	if err != nil {
		panic(err)
	}

	ws.Write(sigStart)
	for {
		ws.Read(buffer)
		//fmt.Println("<RECV>", string(buffer))

		if buffer[16] == 117 {
			var count byte
			count = 48
			status := 0

			i := 31
			for ; buffer[i] != 34; i++ {
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

			if count < 58 {
				ans = countAns1
				ans[10] = count
			} else {
				ans = countAns2
				ans[11] = count - 10
			}
		} else if buffer[16] == 112 {
			ans = ansTemplate
			pointer := 11
			status := 0

			for i := 33; buffer[i] != 34; i++ {
				ans[pointer] = buffer[i]
				pointer++

				if buffer[i] == 109 {
					status = 1
				} else if status == 1 && buffer[i] == 111 {
					status = 2
				} else if status == 2 && buffer[i] == 105 {
					status = 0
					ans[pointer-3] = 77
					ans[pointer-2] = 79
					ans[pointer-1] = 73
				} else {
					status = 0
				}
			}

			ans[pointer] = 34
			ans[pointer+1] = 125
			ans = ans[:pointer+2]
		} else if buffer[16] == 118 {
			ans = ansTemplate
			pointer := 11
			status := 0

			i := 33
			for ; buffer[i] != 34; i++ {
			}
			i--

			for ; buffer[i] != 34; i-- {
				ans[pointer] = buffer[i]
				pointer++

				if buffer[i] == 105 {
					status = 1
				} else if status == 1 && buffer[i] == 111 {
					status = 2
				} else if status == 2 && buffer[i] == 109 {
					status = 0
					ans[pointer-1] = 105
					ans[pointer-3] = 109
				} else {
					status = 0
				}
			}

			ans[pointer] = 34
			ans[pointer+1] = 125
			ans = ans[:pointer+2]
		} else if buffer[16] == 116 {
			ans = ansTemplate
			pointer := 11
			counter := 0

			buffer[26] = 0
			buffer[28] = 0

			for i := 26; buffer[i] != 34; i++ {
				if buffer[i+3] == 109 && buffer[i+4] == 111 && buffer[i+5] == 105 {
					counter = 9
				}
				if counter > 0 {
					if i >= 29 {
						ans[pointer] = buffer[i]
						pointer++
					}
					counter--
				}
			}

			ans[pointer] = 34
			ans[pointer+1] = 125
			ans = ans[:pointer+2]
		} else {
			ws.Close()
			break
		}

		//fmt.Println("<SEND>", string(ans))
		ws.Write(ans)
	}

	fmt.Println("<STOP>", string(buffer))
}
