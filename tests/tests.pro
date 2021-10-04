TEMPLATE = subdirs
CONFIG += ordered
QT -= qt core gui
CONFIG += c++17

SUBDIRS = \
    functional_tests \
    cli_tests \
    benchmark_tests

tests.depends = src
