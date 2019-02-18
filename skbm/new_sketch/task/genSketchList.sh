echo "#ifndef SKETCHLIST_H" > ../sketch/sketchList.h
echo "#define SKETCHLIST_H" >> ../sketch/sketchList.h
for file in `ls ../sketch/ | egrep -v -E "factor.h|sketchList.h|SketchBase.h"`
do
	echo '#include "../sketch/'$file'"' >> ../sketch/sketchList.h
done
echo "#endif" >> ../sketch/sketchList.h