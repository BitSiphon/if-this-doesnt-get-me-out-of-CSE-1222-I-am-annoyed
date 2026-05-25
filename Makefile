# Compiler settings
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra

# Enable secondary expansion to evaluate dependencies dynamically at runtime
.SECONDEXPANSION:

# A macro to find shared utility files.
# We use \b (word boundary) after 'main' to avoid unbalancing Make's parenthesis counter.
shared_srcs = $(filter-out $(shell grep -lE "(int|void)[[:space:]]+main\b" $(1)*.cpp 2>/dev/null), $(wildcard $(1)*.cpp))

# Default target
.PHONY: help
help:
	@echo "Usage: make <folder>/<target>"
	@echo "Example: make http/http_client"

# The core pattern rule
%: %.cpp $$(call shared_srcs,$$(dir $$@))
	@echo "========================================"
	@echo "Building $@"
	@echo "Sources: $^"
	@echo "========================================"
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build complete: $@"
