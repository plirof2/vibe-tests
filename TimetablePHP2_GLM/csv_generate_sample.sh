#!/bin/bash

# Remove old files if exist
rm -f teachers.csv classes.csv timetable.csv

# === TEACHERS ===
# Format: teacher_id,name,max_hours,unavailable_days,color,eidikotita
cat > teachers.csv <<EOL
teacher_id,name,max_hours,unavailable_days,color,eidikotita
T01,John Smith,11,Mon|Wed,blue,Math
T02,Mary Johnson,11,Fri,green,English
T03,James Brown,11,,red,Physics
T04,Linda Davis,11,Tue|Thu,orange,Chemistry
T05,Robert Wilson,11,Wed|Fri,cyan,History
EOL


# === CLASSES ===
# Format: class_id,description
cat > classes.csv <<EOL
class_id,description
A,Class A
B1,Class B1
B2,Class B2
C,Class C
D,Class D
E,Class E
F,Class F
EOL


# === TIMETABLE ===
# Format: class_id,day,slot,teacher_id,course_id
cat > timetable.csv <<EOL
class_id,day,slot,teacher_id,course_id
A,Mon,1,T01,
B1,Mon,3,T02,
B2,Tue,2,T03,
C,Wed,4,T04,
D,Fri,5,T05,
E,Fri,7,T01,
F,Thu,8,T02,
EOL

# === FINISH ===
echo "Sample timetable CSV files created:"
echo " - teachers.csv"
echo " - classes.csv"
echo " - timetable.csv"
