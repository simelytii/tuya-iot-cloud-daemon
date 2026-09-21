BIN:=tuya-daemon
SRC:=$(wildcard *.c)
OBJS:=$(SRC:.c=.o)

CPPFLAGS+=-I$(STAGING_DIR)/usr/include/tuya
CPPFLAGS+=-I$(STAGING_DIR)/usr/include/cjson
CFLAGS+=-Wall

LIBS:=-llink_core -lmiddleware_implementation -lplatform_port -lutils_modules \
      -lubus -lubox -lblobmsg_json -luci -lcjson -largp -lpthread

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN) $(OBJS)
