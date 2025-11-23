.PHONY: all clean rebuild release

BUILD_DIR = build
RELEASE_DIR = $(BUILD_DIR)/release
DEBUG_DIR = $(BUILD_DIR)/debug

all:
	@mkdir -p $(DEBUG_DIR)
	# -B sets the build dir, -S . sets the source to the current folder
	@cmake -B $(DEBUG_DIR) -S . -DCMAKE_BUILD_TYPE=Debug
	@cmake --build $(DEBUG_DIR)
	@echo "Running Debug Build..."
	@./$(DEBUG_DIR)/copypad2

release:
	@mkdir -p $(RELEASE_DIR)
	# We turn off shared libs for static linking of dependencies
	@cmake -B $(RELEASE_DIR) -S . -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
	@cmake --build $(RELEASE_DIR)
	@echo "Release build completed!"
	@otool -L $(RELEASE_DIR)/copypad2

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory."

rebuild: clean all