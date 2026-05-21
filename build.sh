#!/bin/bash
echo "Building Student Management System..."
windres app.rc -O coff -o app.res 2>/dev/null && echo "Icon OK" || echo "Icon skipped"
gcc -o sms_gtk.exe \
    main.c auth.c utils.c dashboard.c student.c \
    exam.c test.c fee.c attendance.c profile.c \
    app.res \
    $(pkg-config --cflags --libs gtk4) \
    -std=gnu11 -Wall -mwindows
[ $? -eq 0 ] && echo "Build successful — sms_gtk.exe ready" || echo "Build FAILED"
