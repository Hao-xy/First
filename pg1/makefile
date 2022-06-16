S=server
C=client

all:$(S)  $(C)

$(S):$(S).c
	gcc $^  -o $@
$(C):$(C).c
	gcc $^  -o $@
clean:
	rm $(S) $(C)  -rf
