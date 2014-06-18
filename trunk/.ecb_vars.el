;; Setup ecb for this source tree
;;
;; to auto-load this on startup, add this to your .emacs file:
;; 
;;(if (file-exists-p ".ecb_vars.el")
;;    (load-file ".ecb_vars.el") )
;;
;; starting emacs in this directory will cause this file to be loaded.
;; then a simple M-x ecb-start will do the rest.
;;

(defun ecb-start ()
  "Setup ecb to run in this source tree"
  (interactive nil)
  (let* ((pwd (getenv "PWD")) ;;Where we started from. Can be anywhere in the source tree
         ;;Project top is where the .git dir is
         (top (find-git-dir) )
         ;; Where do we run gdb, cscope and make from?
         (builddir (concat top ""))
         ;; Source dirs setup. They show up in ecb's browser in the listed order
         (sdirs (list "" ;; top dir
                      ))
         (dirlist (mapcar (lambda(sdir) (concat top sdir)) sdirs) )
         (cvar (list 'ecb-source-path (list 'quote dirlist) ))
         )
    (cscope-set-initial-directory builddir)
    (custom-set-variables cvar)
    ;;(setq gud-gdb-command-name (format "arm-none-eabi-gdb -f --interpreter=mi2 -n -x .gdbinit -cd %S LE320.elf" builddir))
    (print "OK." )
    (ecb-activate) ;;start ecb
    )
  )

;; Iterate up the directory tree looking for a .git dir. This is the project top.
(defun find-git-dir ()
  "Look up till we find a git dir. Report the containing dir."
  (interactive nil)
  (let* (
         (curdir (directory-file-name (getenv "PWD"))) 
         (counter 0 )
         (rval "")
         )
    (while (and (not (file-exists-p  (concat curdir "/.git") ) ) (< counter 10))
      (setq curdir (directory-file-name (file-name-directory curdir)))
      (setq counter (+ counter 1))
      )
    (if (< counter 10) 
      (setq rval curdir) )
    rval
    )
  )
