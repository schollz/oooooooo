s.waitForBoot({
    Routine {
        "running tests".postln;
        n = NetAddr("localhost", 9999);

        // Load audio file
        "load file".postln;
        n.sendMsg("/softcut/buffer/clear");
        n.sendMsg("/softcut/buffer/read_mono", "/home/zns/Documents/oooooooo/amen_beats8_bpm172.wav", 0, 0, -1, 0, 0);
        n.sendMsg("/softcut/buffer/read_mono", "/home/zns/Documents/oooooooo/strega.wav", 0, 30, -1, 0, 0);
        1.wait;

        // Configure and enable voice 0
        2.do({
            arg i;
            n.sendMsg("/set/enabled/cut", i, 1);
            n.sendMsg("/set/level/cut", i, 1.0 + (1.0 * i));
            n.sendMsg("/set/param/cut/rate", i, 41000/48000);
            n.sendMsg("/set/pan/cut", i, 0);
            n.sendMsg("/set/param/cut/buffer", i, 0);
            n.sendMsg("/set/param/cut/loop_start", i, 30*i);
            n.sendMsg("/set/param/cut/loop_end", i, 60/172*4.0+(30*i));
            n.sendMsg("/set/param/cut/loop_flag", i, 1);
            n.sendMsg("/set/param/cut/fade_time", i, 60/172);
            n.sendMsg("/set/param/cut/post_filter_dry", i, 1);
            n.sendMsg("/set/param/cut/play_flag", i, 1);
            n.sendMsg("/set/param/cut/position", i, 0);

        });

        "play_flag".postln;

        // Reverb test settings
        3.wait;
        "reverb on".postln;
        n.sendMsg("/set/reverb/enabled", 1.0);
        n.sendMsg("/set/reverb/mix", 1.0);
        n.sendMsg("/set/reverb/send", 1, 1.0);
        3.wait;
        "reverb off".postln;
        n.sendMsg("/set/reverb/send", 1, 0.0);
        3.wait;
        "tapefx on".postln;
        n.sendMsg("/set/param/cut/tape_pregain", 0, 8.0);
        n.sendMsg("/set/param/cut/tape_bias", 0, 1.0);

        // n.sendMsg("/set/reverb/param/decay", 2.0);
        // n.sendMsg("/set/reverb/param/tail_density", 0.75);
        // n.sendMsg("/set/reverb/param/input_diffusion1", 0.3);
        // n.sendMsg("/set/reverb/param/input_diffusion2", 0.4);
    }.play;
});
