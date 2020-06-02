
function construct_queries(block) {
    var queries=[];
    $.each(block.find(".query-row"), function (ignored, r) {
        var cells=[];
        $.each($(r).find(".query-cell"), function (ignored, c) {
            cells.push($(c).data('control'));
        });
        if (cells.length>0) queries.push(cells);
    });
    return queries;
}

function search(block) {
    var queries=construct_queries(block);
    if (queries.length==0) return;

    var rh=block.find(".result-header");
    var rc=block.find(".result-content");
    rh.addClass("wip");
    apiHost.search(queries).then(function (data) {
        rh.removeClass("wip");
        rc.empty();
        
        $.each(data.response, function (ignored, data1) {
            var segment=$(".template .result-segment").clone();
            segment.find("span").text(data1.name+": "+data1.time.toFixed(3)+"-"+(data1.time+data1.duration).toFixed(3)+"s");
            segment.find("img").attr("src",data1.thumbnail).click(function () {
                $("#playback").data("data", data1);
                $("#playback").foundation("open");
            });
            rc.append(segment);
        });
    }).catch(function (e) {
        rh.removeClass("wip");
        rc.empty();
        console.log(e);
    });
}
