SUB_DIR := ./step1 ./step2 ./step3 ./step4

all:
	@for n in $(SUB_DIR); do $(MAKE) -C $$n || exit 1; done

clean:
	@for n in $(SUB_DIR); do $(MAKE) -C $$n clean || exit 1; done
