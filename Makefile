.PHONY: all clean rebuild release

BUILD_DIR = build

all: $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Debug || exit 1
	@cd $(BUILD_DIR) && cmake --build . || exit 1
	@echo "Build completed successfully!"
	@cd $(BUILD_DIR) && ./copypad2

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Build directory cleaned"

release:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DCMAKE_EXE_LINKER_FLAGS="-static" || exit 1
	@cd $(BUILD_DIR) && cmake --build . || exit 1
	@echo "Static build completed successfully!"
	@cd $(BUILD_DIR) && otool -L ./copypad2

rebuild: clean all
