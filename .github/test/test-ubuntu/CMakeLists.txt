cmake_minimum_required(VERSION 3.0)

project(arrp-test)

find_package(Arrp REQUIRED)

arrp_to_exe(arrp-test ../test.arrp)
