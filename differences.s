; tabulate a polynomial by the method of differences

        $0 $main .
a0:     1                                                                    
a1:     -1                                                                   
a2:     1                                                                    
a3:     -1                                                                   
a4:     1                                                                    
a5:     -1                                                                   
tmp:    0                                                                    
main:                                                                       
        $0 $a5 @ - $tmp ! $a4 @ $tmp @ - $a5 !                               
        $0 $a4 @ - $tmp ! $a3 @ $tmp @ - $a4 !                               
        $0 $a3 @ - $tmp ! $a2 @ $tmp @ - $a3 !                               
        $0 $a2 @ - $tmp ! $a1 @ $tmp @ - $a2 !                               
        $0 $a1 @ - $tmp ! $a0 @ $tmp @ - $a1 !
        ; $a5 @ $arg1 !                                                        
        ; $0 $output .          # we don't have an output routine yet
        ; Instead, ./cavosim differences.image| grep 'mem\[8]' | head 
        $0 $main .                                                           
