CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2

TARGET1 = most_sem
TARGET2 = most_cond
TARGETS = $(TARGET1) $(TARGET2)

PREFIX = /usr/local/bin

all: $(TARGETS)

$(TARGET1): $(TARGET1).c
	$(CC) $(CFLAGS) $(TARGET1).c -o $(TARGET1)

$(TARGET2): $(TARGET2).c
	$(CC) $(CFLAGS) $(TARGET2).c -o $(TARGET2)

clean:
	rm -f $(TARGETS)

install: $(TARGETS)
	@cp $(TARGET1) $(PREFIX)/$(TARGET1)
	@chmod 755 $(PREFIX)/$(TARGET1)
	@echo "install: $(PREFIX)/$(TARGET1)."
	@cp $(TARGET2) $(PREFIX)/$(TARGET2)
	@chmod 755 $(PREFIX)/$(TARGET2)
	@echo "install: $(PREFIX)/$(TARGET2)."

uninstall:
	@rm -f $(PREFIX)/$(TARGET1)
	@echo "uninstall: $(PREFIX)/$(TARGET1)."
	@rm -f $(PREFIX)/$(TARGET2)
	@echo "uninstall: $(PREFIX)/$(TARGET2)."

.PHONY: all clean install uninstall