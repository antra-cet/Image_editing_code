build: image_editor.c functions.c
	gcc image_editor.c functions.c -o image_editor -Wall -Wextra
clean:
	rm -rf image_editor
