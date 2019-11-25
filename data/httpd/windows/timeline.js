function wndTimeline(wnd){
    this._pWindow = wnd;
    this._pCanvas = this._pWindow.find(".bgCanvas");
    

    var canvasCtx = this._pCanvas[0].getContext("2d");

var ratio = window.devicePixelRatio;
var canvas = this._pCanvas[0];
var oldWidth = canvas.width;
        var oldHeight = canvas.height;

        canvas.width = oldWidth * ratio;
        canvas.height = oldHeight * ratio;

        canvas.style.width = oldWidth + 'px';
        canvas.style.height = oldHeight + 'px';

        // now scale the context to counter
        // the fact that we've manually scaled
        // our canvas element
        canvasCtx.scale(ratio, ratio);

    var col_1 = "rgb(91,91,91)";
    var col_2 = "rgb(95,95,95)";
    var col_l1= "rgb(80,80,80)";
    var col_l2= "rgb(97,97,97)";

    canvasCtx.fillStyle = col_1;
    canvasCtx.fillRect(0, 0, 100, 32);
    canvasCtx.fillStyle = col_2;
    canvasCtx.fillRect(0, 32, 100, 32);
canvasCtx.lineWidth = 1.0;

canvasCtx.strokeStyle = col_l1;
canvasCtx.beginPath();
canvasCtx.moveTo(0, 32);
canvasCtx.lineTo(100, 32);
canvasCtx.stroke();

canvasCtx.strokeStyle = col_l1;
canvasCtx.beginPath();
canvasCtx.moveTo(0, 0);
canvasCtx.lineTo(100, 0);
canvasCtx.stroke();


canvasCtx.strokeStyle = col_l2;
canvasCtx.beginPath();
canvasCtx.moveTo(50, 0);
canvasCtx.lineTo(50, 64);
canvasCtx.stroke();

canvasCtx.strokeStyle = col_l2;
canvasCtx.beginPath();
canvasCtx.moveTo(0, 0);
canvasCtx.lineTo(0, 64);
canvasCtx.stroke();


$(".tWrapper").css({"background-image":canvas.toDataURL(),"background-repeat":"true true"});

}


wndShaders.prototype.resize = function(){

}