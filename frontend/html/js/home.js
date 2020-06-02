$("#pg-home").on(":initpage", function () {
    var page=$(this);

    apiHost.setting().then(function (data) {
        var control_panel=page.find(".control-panel");
        $.each(data.controls, function (ignored, control) {
            var div=$(".template .control-item").clone();
            div.find("img").attr("src",control.icon);
            div.find(".control-text").text(control.description);
            div.find(".query-cell").data("control", control);
            control_panel.append(div.children());
        });

        /* drag and drop controls to search */
        $.each(control_panel.find(".control-icon"), function (ignored, moveable) {
            Sortable.create(moveable, {
                group: {
                    "name": "search-constructor",
                    "pull": "clone",
                    "put": false,
                },
                sort: false,
                draggable: ".query-cell",
                onClone: function (e) {
                    var control=$(e.item).data('control');
                    var control2=JSON.parse(JSON.stringify(control));
                    $(e.clone).data('control',control2);
                },
            });
        });
    });

    var setup_query_row=function (ignored, cell) {
        var instance=Sortable.get(cell);
        if (typeof(instance)!=="undefined") return;

        var cleanup_queries=function (body) {
            /* make sure there is always an empty query-row */
            body.find(".query-row:not(:has(*))").remove();
            var div=$('<div class="query-row"></div>');
            setup_query_row(0,div[0]);
            body.find(".query-content").append(div);
        };

        Sortable.create(cell, {
            group: {
                "name": "search-constructor",
                "pull": true,
                "put": true,
            },
            sort: true,
            draggable: ".query-cell",
            removeOnSpill: true,
            swapThreshold: 0.5,
            onAdd: function (e) {
                var item=$(e.item);
                item.unbind('click').click(function () {
                    var setting=$("#query-setting");
                    setting.data("control", item.data('control'));
                    $("#query-setting").foundation("open");
                });
                setTimeout(cleanup_queries,100,item.parents(".query-body"));
            },
            onSpill: function (e) {
                var item=$(e.item);
                setTimeout(cleanup_queries,100,item.parents(".query-body"));
            },
        });
    };

    var setup_query_block=function (workspace) {
        var block=$(".template .query-block").clone();
        $.each(block.find(".query-row"), setup_query_row);
        block.find(".result-header").click(function () {
            block.find(".result-content").empty();
            search(block);
        });
        block.find(".query-header").click(function () {
            block.remove();
        })
        workspace.append(block);
    };

    /* setup search workspace */
    var workspace=page.find(".query-panel");
    workspace.empty();
    setup_query_block(workspace);

    workspace.on('dblclick', function () {
        setup_query_block(workspace);
    });
});
