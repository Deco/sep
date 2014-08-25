git subtree pull --squash \
    --prefix dependencies/msgpack-c/ \
    https://github.com/msgpack/msgpack-c.git master \
    --message="Updating dependencies/msgpack-c" \
;

git subtree pull --squash \
    --prefix dependencies/libevent/ \
    https://github.com/libevent/libevent.git master \
    --message="Updating dependencies/libevent" \
;
