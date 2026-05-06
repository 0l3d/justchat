# justchat.
Simple and Minimalist chat protocol and application (self-hostable).  
  
## Installation & Usage 
  
Install from Github Releases or build yourself.  
Static Release deps: `NONE`  
Dynamic Release deps: `libwebsockets` and `libwebsockets's deps`  
  
### Build
deps: `libwebsockets`  
```
git clone https://git.sr.ht/~oled/justchat
cd justchat/
make # dynamic or static
```   
  
### Usage 
  
```
$ ls 
justchat. 
page/index.html
page/404.html
$ ./justchat.
```  
Then websocket is available on 8080 port!  
  
## Protocol Specs
  
#### 1. POSIX Frame (Terminal)
- **Transport:** TCP / `select(2)`
- **Format:** `<username>,<message>\n`
- **Limits:** User: 31 chars | Msg: 900 bytes
  
#### 2. Web Frame (Websocket)
- **Transport:** `libwebsockets`
- **Format:** `<channel>,<username>,<message>`
- **Logic:** Broadcasts only to clients in the same `channel_name`.
- **Limits:** Channel: 32 chars | User: 32 chars | Msg: 900 bytes
  
#### Constraints
- **Delimiter:** `,` (Comma) is reserved for field separation.
- **Terminator:** `\n` is required for POSIX; Websockets use frame-based delivery.
