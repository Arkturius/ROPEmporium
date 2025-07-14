#
# ARM Docker environment Makefile
#

DIR		:= ROPEmporium
NAME	:= armv5-ctf

build:
	@docker build -t $(NAME) . 

run:
	@docker run \
		-it --rm --cap-add=SYS_PTRACE \
		-v $(PWD)/..:/$(DIR) -w /$(DIR) $(NAME) \
		bash -c "cd tools && pip install -e . && cd ../ARM && bash"

clean:
	for chall in $(ls -d */); do cd $$chall && rm -rf core* qemu_* && cd ..; done
