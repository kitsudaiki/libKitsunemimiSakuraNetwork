TEMPLATE = subdirs
CONFIG += ordered
QT -= qt core gui
CONFIG += c++14

SUBDIRS = \
    functional_tests \
    cli_tests

tests.depends = src
