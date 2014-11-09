
MODE=${1:-pull}

git subtree $MODE --squash \
    --prefix dependencies/websocketpp/websocketpp/ \
    https://github.com/zaphoyd/websocketpp.git master \
    --message="Updating dependencies/websocketpp" \
;

git subtree $MODE --squash \
    --prefix dependencies/rapidjson/rapidjson/ \
    https://github.com/miloyip/rapidjson.git master \
    --message="Updating dependencies/rapidjson" \
;

