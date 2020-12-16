BIN:=luci_watson_web_prog

.PHONY: all clean

CFLAGS += -Wall -std=gnu99
LDFLAGS := -liotfdeviceclient -liotfgatewayclient -liotfdevicemgmt -luci

all: $(BIN)

$(BIN): luci_watson_web_prog.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(BIN)

