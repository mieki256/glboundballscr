glboundball.scr: glboundball.o resource.o
	g++ glboundball.o resource.o -o glboundball.scr -static -lstdc++ -lgcc -lscrnsave -lopengl32 -lglu32 -lgdi32 -lcomctl32 -lwinmm -mwindows

glboundball.o: glboundball.cpp
	g++ -c glboundball.cpp

resource.o: resource.rc resource.h icon.ico
	windres resource.rc resource.o

.PHONY: cleanall
cleanall:
	rm -f *.scr *.o

.PHONY: clean
clean:
	rm -f *.o
