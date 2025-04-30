s.waitForBoot({
    Routine{
        "running tests".postln;
        n = NetAddr("localhost", 9999);
        "load file".postln;
        f = "/home/zns/Documents/oooooooo/strega.wav";
        n.sendMsg("/softcut/buffer/clear");
        n.sendMsg("/softcut/buffer/read_mono", f, 0, 0, 10, 0, 0);
        1.wait;
        n.sendMsg("/set/enabled/cut", 0, 1);
        n.sendMsg("/set/level/cut", 0, 2.0);
        n.sendMsg("/set/pan/cut", 0, 0);

        n.sendMsg("/set/param/cut/buffer", 0, 0);

        n.sendMsg("/set/param/cut/loop_start", 0, 0);
        n.sendMsg("/set/param/cut/loop_end", 0, 3);
        n.sendMsg("/set/param/cut/loop_flag", 0, 1);

        n.sendMsg("/set/param/cut/fade_time", 0, 1);

        n.sendMsg("/set/param/cut/post_filter_lp", 0, 1);  
        n.sendMsg("/set/param/cut/post_filter_hp", 0, 0);
        n.sendMsg("/set/param/cut/post_filter_bp", 0, 0);
        n.sendMsg("/set/param/cut/post_filter_br", 0, 0);
        n.sendMsg("/set/param/cut/post_filter_dry", 0, 1);
        n.sendMsg("/set/param/cut/play_flag", 0, 1);
    	n.sendMsg("/set/param/cut/position", 0, 0);
        "play_flag".postln;
    }.play;

});