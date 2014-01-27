

open Printf;;
open Images;;


let get_size args =
        (int_of_string args.(1), int_of_string args.(2))
;;


let () = 
        printf "Width:\t%d\n" (fst (get_size Sys.argv));
        printf "Height:\t%d\n" (snd (get_size Sys.argv))
;;

let w = fst (get_size Sys.argv);;
let h = snd (get_size Sys.argv);;

let gen_mat w h c







