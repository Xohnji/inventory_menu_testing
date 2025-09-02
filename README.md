git clone https://github.com/Xohnji/inventory_menu_testing.git
git clone https://github.com/Microsoft/vcpkg.git

./bootstrap-vcpkg.bat
# For MinGW/GCC:
./vcpkg install curl jsoncpp --triplet x64-mingw-dynamic

# For Visual Studio (MSVC):
./vcpkg install curl jsoncpp --triplet x64-windows

./vcpkg integrate install
