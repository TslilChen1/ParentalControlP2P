# הגדרת גרסת מינימום של CMake
cmake_minimum_required(VERSION 3.10)

# הגדרת שם הפרויקט
project(VP8-forImage)

# הגדרת המיקום של ספריית libvpx במחשב שלך
set(LIBVPX_DIR "C:/Users/Magshimim/Documents/Magshimim/or-akiva-504-parentalcontrolp2p/libvpx")

# הוספת ספריות כותרות (headers) של libvpx
include_directories(${LIBVPX_DIR}/include)

# הגדרת קובץ המקורות (אם יש קובץ בשם main.cpp, תשנה את השם בהתאם)
set(SOURCE_FILES main.cpp)

# הוספת קובץ ההרצה של הפרויקט
add_executable(VP8-forImage ${SOURCE_FILES})

# קישור לספריית libvpx
target_link_libraries(VP8-forImage ${LIBVPX_DIR}/libvpx.a)

# במידה ויש קבצי DLL או ספריות נוספות (בהתאם למערכת), עליך להוסיף קבצים נוספים
# אם אתה משתמש ב-Windows, ייתכן שיהיה צורך להוסיף גם את ספריית ה-bin
# לדוגמה:
# link_directories(${LIBVPX_DIR}/bin)
