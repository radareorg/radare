@bash --version 2> nul > nul
@if errorlevel 1 goto end
@bash install.sh

@goto realend
@:end

@echo *****
@echo Error: Cannot find bash in PATH
@echo *****
@pause

@:realend
