SHELL := /bin/bash

.PHONY: help setup-env

help:
	@echo "FamilyBlock Agent commands"
	@echo ""
	@echo "  make setup-env   Create local .env from .env.example if missing"
	@echo ""
	@echo "Build and install the Windows service with Visual Studio on a Windows machine."
	@echo "See README.md and DEPLOYMENT.md for configuration details."

setup-env:
	@test -f .env || cp .env.example .env
	@echo "Local .env is present. Fill backend URL and device credentials before running the service."
