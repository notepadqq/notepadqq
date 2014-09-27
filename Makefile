.PHONY: ___default_target %

# Handle calls to `make`
___default_target:
	$(MAKE) -C src/ui

# Handle calls to `make <target>`
%:
	$(MAKE) -C src/ui "$@"