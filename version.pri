unix:system(which git): HAS_GIT = TRUE
win32:system(where git.exe): HAS_GIT = TRUE
contains(HAS_GIT, TRUE) {
    GIT_VERSION = $$system(git describe --always --dirty --long --tags)
}
isEmpty(GIT_VERSION): GIT_VERSION = "(unknown version)"

DEFINES += "QDB_VERSION=\\\"$$GIT_VERSION\\\""
