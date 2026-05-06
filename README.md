# justchat.
Simple and Minimalist chat protocol and application (self-hostable).  

<img width="642" height="192" alt="image" src="https://github.com/user-attachments/assets/13745590-833f-4b30-b5d5-bf6426340673" />
<img width="1365" height="745" alt="image" src="https://github.com/user-attachments/assets/658e60d0-5723-4606-889e-6e1b51e22f01" />


## Installation & Usage 
  
Install from Github Releases or build yourself.  
Static Release deps: `NONE`  
Dynamic Release deps: `libwebsockets` and `libwebsockets's deps`   
  
**INFO**: Gonna compile libwebsockets from scratch? install deps first. Or just get it from your package manager, it handles all dependencies for you.
  
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
justchat (dynamic or static)
page/index.html
page/404.html
$ ./justchat (dynamic or static)
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
