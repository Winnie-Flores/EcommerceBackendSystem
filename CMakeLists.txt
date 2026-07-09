cmake_minimum_required(VERSION 3.16)
project(ECommerce VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 5 (Anaconda)
set(QT_PATH /opt/anaconda3)
set(CMAKE_PREFIX_PATH ${QT_PATH})
find_package(Qt5 REQUIRED COMPONENTS Widgets Sql Charts)

# MySQL
set(MYSQL_INCLUDE_DIR /usr/local/mysql/include)
set(MYSQL_LIB_DIR /usr/local/mysql/lib)
set(OPENSSL_INCLUDE_DIR /opt/homebrew/opt/openssl/include)
set(OPENSSL_LIB_DIR /opt/homebrew/opt/openssl/lib)
include_directories(${MYSQL_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
link_directories(${MYSQL_LIB_DIR} ${OPENSSL_LIB_DIR})

# Sources
set(SOURCES
    src/main.cpp
    src/dbmanager.cpp
    src/login_dialog.cpp
    src/main_window.cpp
    src/user_page.cpp
    src/product_page.cpp
    src/supplier_page.cpp
    src/inventory_page.cpp
    src/order_page.cpp
    src/report_page.cpp
    src/profile_page.cpp
    src/supplier_product_page.cpp
    src/supplier_order_page.cpp
    src/supplier_analysis_page.cpp
)

set(HEADERS
    include/dbmanager.h
    include/login_dialog.h
    include/main_window.h
    include/user_page.h
    include/product_page.h
    include/supplier_page.h
    include/inventory_page.h
    include/order_page.h
    include/report_page.h
    include/profile_page.h
    include/supplier_product_page.h
    include/supplier_order_page.h
    include/supplier_analysis_page.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    Qt5::Sql
    Qt5::Charts
    mysqlclient
    ssl
    crypto
)

# 设置 OpenSSL 动态库搜索路径
set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_RPATH "${OPENSSL_LIB_DIR}"
    INSTALL_RPATH "${OPENSSL_LIB_DIR}"
)

# 编译后自动修复 mysqlclient 间接依赖的 ssl/crypto 动态库路径
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND install_name_tool -change libssl.3.dylib ${OPENSSL_LIB_DIR}/libssl.3.dylib $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND install_name_tool -change libcrypto.3.dylib ${OPENSSL_LIB_DIR}/libcrypto.3.dylib $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/style.qss
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/style.qss
    COMMENT "Fixing OpenSSL dylib paths and copying QSS"
)
