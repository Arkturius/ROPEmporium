NAME	:= armv5-ctf

build:
	@docker build -t $(NAME) . 

run:
	@docker run -it --rm --cap-add=SYS_PTRACE -v $(PWD)/..:/ctf -w /ctf $(NAME) bash -c "pip install -e . && bash"
