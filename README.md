# Send/receive messages with GSM module
Two simple programs that sends and receives SMS messages with AT commands.

# Sending
* Package 'gsm_send' sends SMS messages. Compile from directory with 'make'.
* Two parameters have to be passed: the phone number (national conventions without + sign at the moment) and the message.
* The program has to be started with root privileges and it checks '/dev/ttyUSB2' port at the moment. If it doesn't work, try changing to another port.
* Multiple word message can be sent by wrapping the second parameter with quotation marks.
* Works with UNICODE alphabet.

# Receiving
* Package 'gsm_receive' receives SMS messages. Compile from directory with 'make'.
* The program periodically sends a command to check if there are any unread messages (every 1 second) and prints out a message if its available.
* The program has to be started with root privileges and it checks '/dev/ttyUSB2' port at the moment. If it doesn't work, try changing to another port.

# Named pipes
* Package 'named_pipes' contains two directories with simmilar programs above. Compiling is the same.
* Launching both programs on separate terminals and sending a SMS message to the modem will show communication between two processes.a

# Credits
* 'gsm_send' program uses tools from [SMS Server Tools 3](http://smstools3.kekekasvi.com/). All credit to Stefan Frings (original author of SMS Server Tools), Keijo "Keke" Kasvi (current maintainer) and other contributors.
* 'gsm_receive' program uses tools from [hitmoon pdu decoder](https://github.com/hitmoon/sms-pdu).