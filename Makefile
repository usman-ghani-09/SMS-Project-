CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11 $(shell pkg-config --cflags gtk4)
LDFLAGS = $(shell pkg-config --libs gtk4) -mwindows
TARGET  = sms_gtk
SRCS    = main.c auth.c utils.c dashboard.c student.c exam.c test.c fee.c profile.c attendance.c

all: $(TARGET)
app.res: app.rc app.ico
	windres app.rc -O coff -o app.res

$(TARGET): $(SRCS) app.res
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) app.res $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET).exe app.res tmp_*.dat

run: all
	./$(TARGET)
