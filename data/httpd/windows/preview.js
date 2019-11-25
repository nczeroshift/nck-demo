
function wndPreview(wnd){
    this._pWindow = wnd;
    this._pTime = 1000;

    var canvasW = this._pWindow.find(".mainCanvas");
    var canvas = canvasW[0];
    var width = canvasW.width();
    var height = canvasW.height();
    
    this._pCanvasW = canvasW;

    canvasW.attr({width:width,height:height});
    var context = canvas.getContext("2d");
    var id = 0;

    var scope = this;
    //context.scale(0.25,0.25)
    function paintImage(){
        var img = new Image();
        
        img.onload = function(){
            width = canvasW.width();
            height = canvasW.height();
            var scale = (width / img.width)*0.95;
            var w = scale * img.width;
            var h = scale * img.height;
            var x = (width - w)*0.5;
            var y = (height - h)*0.5;
            

            context.clearRect(0, 0, width, height);
            context.drawImage(img,x,y,w,h);
            setTimeout(paintImage,scope._pTime);
        }
        
        img.onerror = function(){
            setTimeout(paintImage,scope._pTime);
        }

        img.src="/x/data.jpg?screenshot="+id;
        id+=1;
    }

    /*$(window).resize(function(){
        var width = canvasW.width();
        var height = canvasW.height();
        canvasW.attr({width:width,height:height});
    });*/
    
    this._pWindow.find(".preview_rate").change(function(){
        scope._pTime = parseInt($(this).val());
    })
    paintImage();

}

wndPreview.prototype.resize = function(){
    var width = this._pCanvasW.width();
    var height = this._pCanvasW.height();
    this._pCanvasW.attr({width:width,height:height});
}