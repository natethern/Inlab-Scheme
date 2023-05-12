;;; (c) Copyright 1991-2002,2003 by Inlab Software GmbH, Gruenwald
;;; All Rights Reserved / Alle Rechte vorbehalten
;;; $Id: init.scm,v 4.108 2006/04/29 09:20:33 tommy Exp $

(define 
  scm-init-rcsid 
  "$Id: init.scm,v 4.108 2006/04/29 09:20:33 tommy Exp $")

;;; list

(define 
  list 
  (named-lambda (list . x) x))

;;; aus (lambda x y)  ->  (named-lambda (() x) y)

(define lambda 
  (named-syntax-lambda (lambda expr env)
       (cons
         'named-lambda
         (cons (cons '() (car (cdr expr)))
               (cdr (cdr expr))))))
     
(define syntax-lambda 
  (named-syntax-lambda (syntax-lambda expr env)
       (cons
         'named-syntax-lambda
         (cons (cons '() (car (cdr expr)))
               (cdr (cdr expr))))))

;;; Initialisierung von rand 

(srand (get-time))	  		

;;;
;;; cadr and friends
;;;

(define (cadr x) (car (cdr x)))
(define (caar x) (car (car x)))
(define (cdar x) (cdr (car x)))
(define (cddr x) (cdr (cdr x)))

(define (caaar x) (car (car (car x))))
(define (caadr x) (car (car (cdr x))))
(define (cadar x) (car (cdr (car x))))
(define (caddr x) (car (cdr (cdr x))))
(define (cdaar x) (cdr (car (car x))))
(define (cdadr x) (cdr (car (cdr x))))
(define (cddar x) (cdr (cdr (car x))))
(define (cdddr x) (cdr (cdr (cdr x))))

(define (caaaar x) (car (caaar x)))
(define (caaadr x) (car (caadr x)))
(define (caadar x) (car (cadar x)))
(define (caaddr x) (car (caddr x)))
(define (cadaar x) (car (cdaar x)))
(define (cadadr x) (car (cdadr x)))
(define (caddar x) (car (cddar x)))
(define (cadddr x) (car (cdddr x)))
(define (cdaaar x) (cdr (caaar x)))
(define (cdaadr x) (cdr (caadr x)))
(define (cdadar x) (cdr (cadar x)))
(define (cdaddr x) (cdr (caddr x)))
(define (cddaar x) (cdr (cdaar x)))
(define (cddadr x) (cdr (cdadr x)))
(define (cdddar x) (cdr (cddar x)))
(define (cddddr x) (cdr (cdddr x)))

;;;
;;; interne Funktionen, die von Macros verwendet werden
;;;

(define (scm-append x y)
  (if (null? x) 
    y
    (cons (car x)
      (scm-append (cdr x) y))))

(define (scm-variables expr)
  (if (null? expr) ()
    (cons (caar expr) 
      (scm-variables (cdr expr)))))

(define (scm-values expr)
  (if (null? expr) ()
    (cons 
      (if (pair? (cdar expr))
	(cadar expr) 
        '(get-unassigned-constant))	;;; unassigned
      (scm-values (cdr expr)))))

(define let 
  (named-syntax-lambda (let expr env)
    (if (symbol? (cadr expr))		
      (cons 'internal-named-let 
	(cdr expr))
      (cons 
        (scm-append 
          (list 
            'lambda (scm-variables (cadr expr))) 
	  (cddr expr))
        (scm-values (cadr expr))))))

;;;
;;; linkassoziative Expansion
;;;

(define (scm-two-arg-expand-left expr old-symbol new-symbol no-arg-value)
  (define args (cdr expr))
  (if (null? args) 
    (list 'quote no-arg-value))
    (if (null? (cdr args)) 
      (list new-symbol no-arg-value (car args))
      (if (null? (cddr args)) 
	(list new-symbol (car args) (cadr args))
	(append
	  (list old-symbol (list new-symbol (car args) (cadr args)))
	  (cddr args)))))
		    
;;;

(define (+ . e)
  (define (addall l)
    (if (null? l)
	0
	(internal-+ (car l) (addall (cdr l)))))
  (addall e))

(define (- . e)
  (define (addall l)
    (if (null? l)
	0
	(internal-+ (car l) (addall (cdr l)))))
  (define (s l)
    (if (null? l)
        0
        (if (null? (cdr l))
	    (internal-- 0 (car l))
            (internal-- (car l) (addall (cdr l))))))
    (s e))

(define (* . e)
  (define (mulall l)
    (if (null? l)
	1
	(internal-* (car l) (mulall (cdr l)))))
  (mulall e))

;;; append

(define (append . e)
  (define r '())
  (define (fe l proc)
    (if (null? l)
        #t
        (begin
          (proc (car l))
          (fe (cdr l) proc))))
  (fe e (lambda (x)
           (set! r (scm-append r x))))
  r)

(define (string-append . e)
  (define r (fixarg-make-string 0 #\space))
  (define (fe l proc)
    (if (null? l)
        #t
        (begin
          (proc (car l))
          (fe (cdr l) proc))))
  (fe e (lambda (x)
           (set! r (internal-string-append r x))))
  r)

;;;; and nicht ganz korrekt: Hier wird nicht der Wert des letzten 
;;;; wahren Praedikats zurueckgegeben, sondern #t ...
;;;;
;;;; Trotzdem wird es hier erst einmal definiert (um dann spaeter
;;;; richtig definiert zu werden...)

(define internal-and 
  (named-syntax-lambda (ia expr env)
    (if (null? (cdr expr))
	''#t
	(list 'if 
	      (list 'not (cadr expr))
	      ''#f
	      (cons 'internal-and (cddr expr))))))

(define (char-upper-case? char)
  (internal-and (char>=? char #\A) (char<=? char #\Z)))

(define (char-lower-case? char)
  (internal-and (char>=? char #\a) (char<=? char #\z)))

(define (char-numeric? char)
  (internal-and (char>=? char #\0) (char<=? char #\9)))

(define (char-alphabetic? char)
  (or (char-upper-case? char) (char-lower-case? char)))

(define (char-whitespace? char)
  (or (char=? char #\space) (char=? char #\newline)))

(define (char-upcase char)
  (if (char-lower-case? char)
      (integer->char (- (+ (char->integer char) (char->integer #\A)) 
			(char->integer #\a)))
      char))

(define (char-downcase char)
  (if (char-upper-case? char)
      (integer->char (- (+ (char->integer char) (char->integer #\a)) 
			(char->integer #\A)))
      char))

(define (char-ci=? c1 c2) 
  (char=? (char-downcase c1) (char-downcase c2)))

(define (char-ci<? c1 c2) 
  (char<? (char-downcase c1) (char-downcase c2)))

(define (char-ci>? c1 c2) 
  (char>? (char-downcase c1) (char-downcase c2)))

(define (char-ci<=? c1 c2) 
  (char<=? (char-downcase c1) (char-downcase c2)))

(define (char-ci>=? c1 c2) 
  (char>=? (char-downcase c1) (char-downcase c2)))

(define (string<=? ss1 ss2) 
  (not (string>? ss1 ss2)))

(define (string>=? ss1 ss2) 
  (not (string<? ss1 ss2)))

(define (string-ci=? ss1 ss2) 
  (string=? (string-downcase ss1) (string-downcase ss2)))

(define (string-ci<? ss1 ss2) 
  (string<? (string-downcase ss1) (string-downcase ss2)))

(define (string-ci>? ss1 ss2) 
  (string>? (string-downcase ss1) (string-downcase ss2)))

(define (string-ci>=? ss1 ss2) 
  (string>=? (string-downcase ss1) (string-downcase ss2)))

(define (string-ci<=? ss1 ss2) 
  (string<=? (string-downcase ss1) (string-downcase ss2)))

(define (system . s) 
  (if (null? s)
    (internal-system "/bin/sh")
    (internal-system (car s))))

(define the-current-editor "vi")
(define the-current-filename ())		;;; wird auch von (load)
						;;; benutzt !
(define (edit . args)
  (if (null? args)
    (begin
      (if (null? the-current-filename)
	  (error "no current filename / edit"))
      (system (string-append the-current-editor " '" the-current-filename "'")))
    (begin (set! the-current-filename (car args))
	   (system (string-append the-current-editor " '" 
				  the-current-filename "'")))))

(define (vector . args) (list->vector args))
(define (string . args) (list->string args))

(define (string-copy s) (substring s 0 (string-length s)))

(define (error s . ignored)
  (internal-error s))

(define (internal-member obj l compare)
  (if (null? l) #f
      (if (not (pair? l))
	#f
        (if (compare obj (car l)) 
	    l
	    (internal-member obj (cdr l) compare)))))

(define (member obj l) (internal-member obj l equal?))
(define (memq obj l) (internal-member obj l eq?))
(define (memv obj l) (internal-member obj l eqv?))

(define (internal-assoc obj l compare)
  (if (null? l) 
      #f
      (if (compare obj (caar l)) 
	  (car l))
	  (internal-assoc obj (cdr l) compare)))

(define (assoc obj l) (internal-assoc obj l equal?))
(define (assq obj l) (internal-assoc obj l eq?))
(define (assv obj l) (internal-assoc obj l eqv?))

(define let* (named-syntax-lambda (let* expr env)
  (let ((binding-list (cadr expr)) (body-list (cddr expr)))
    (if (null? binding-list)
	  (list (cons 'lambda (cons '() body-list))) 
	  (append (list 'let (list (car binding-list))) 
	     (list (cons 'let* (cons (cdr binding-list) body-list))))))))

;;;
;;; QuasiQuote
;;;

(define (internal-qq e o-level c-level)
  (if (eqv? c-level 0)
      (error "level-mismatch / quasiquote")
      (if (pair? e)
	   (if (eq? (car e) 'unquote)
	       (if (eqv? o-level c-level)
		   (cadr e)
		   (list 'cons (internal-qq (car e) o-level (- c-level 1))
			       (internal-qq (cdr e) o-level (- c-level 1))))
	       (if (internal-and (pair? (car e)) (eq? (caar e) 'unquote-splicing))
		   (if (eqv? o-level c-level)
		       (list 'append (cadar e)
				     (internal-qq (cdr e) o-level c-level))
		       (list 'cons (internal-qq (car e) o-level (- c-level 1))
				   (internal-qq (cdr e) o-level (- c-level 1))))
		   (if (eq? (car e) 'quasiquote)
		       (list 'cons (internal-qq (car e) o-level (+ c-level 1))
				   (internal-qq (cdr e) o-level (+ c-level 1)))
		       (list 'cons (internal-qq (car e) o-level c-level)
				   (internal-qq (cdr e) o-level c-level)))))
	  (begin
	    (if (internal-and (not (pair? e)) (not (vector? e)))
		(list 'quote e)
		(begin
		   (if (vector? e)
		       (list 'list->vector (internal-qq (vector->list e) o-level c-level))
		       (if (string? e)
			   (list 'quote e)
			   (if (char? e)
			       (list 'quote e)
			       (error "invalid type / quasiquote"))))))))))

(define quasiquote 
  (named-syntax-lambda (quasiquote expr env) 
	  (internal-qq (cadr expr) 1 1)))

;;;
;;; Error-Generierung bei , und ,@
;;;

(define (unquote . ignored) 
  (error "invalid unquote"))

(define (unquote-splicing . ignored) 
  (error "invalid unquote-splicing"))

;;; 
;;; and 
;;;

(define 
  and 
  (named-syntax-lambda (and expr env)
  (if (null? (cdr expr)) 
      '#t
      (if (null? (cddr expr)) 
	  (cadr expr)
	  `(let ((x ,(cadr expr))
		     (thunk (lambda () (and ,@(cddr expr)))))
	       (if x (thunk) x))))))

(define 
  or 
  (named-syntax-lambda (or expr env)
  (if (null? (cdr expr)) 
      '#f  
      (if (null? (cddr expr)) 
	  (cadr expr)
	  `(let ((x ,(cadr expr))
		     (thunk (lambda () (or ,@(cddr expr)))))
	       (if x x (thunk)))))))

(define letrec (named-syntax-lambda (letrec expr env)
  (define (map1 proc liste)
    (if (null? liste) 
	(quote ())
        (cons (proc (car liste))
              (map1 proc (cdr liste)))))
  (define (map2 proc a b)
    (if (null? a) 
	(quote ())
        (cons (proc (car a) (car b))
	      (map2 proc (cdr a) (cdr b)))))
  (define (make-vlist var)
    `(,var (get-unassigned-constant)))		;;; unassigned
  (define (make-set-list var val)
    `(set! ,var ,val))
  `(let ,(map1 make-vlist (scm-variables (cadr expr)))
    ,@(map2 make-set-list (scm-variables (cadr expr)) 
	    (scm-values (cadr expr)))
    ,@(cddr expr))))

;; von let aus erfolgt ggf. die Umsetzung in internal-named-let:

(define internal-named-let 
  (named-syntax-lambda (internal-named-let expr env)
  `((letrec ((,(cadr expr) (lambda ,(scm-variables (caddr expr))
				,@(cdddr expr)			; body
				)))
	   ,(cadr expr))
      ,@(scm-values (caddr expr)))))

;;;
;;; cond
;;;

(define cond (named-syntax-lambda (cond expr env)
  (if (null? (cdr expr))
      ()
      (begin
	(define test (cadr expr))
	(define clause2 (cddr expr))		; in clause2 Liste
	(if (not (pair? test))
	    (error "syntax-error / cond")
	    (if (null? (cdr test))		; (cond (test) clause2 ..)      
		`(or ,@test (cond ,@clause2)) 
		(if (eq? (cadadr expr) '=>)
		    `(let ((test-result ,(caadr expr))
			  (thunk2 
			    (lambda () ,(car (cdr (cdr (car (cdr expr)))))))
			  (thunk3 (lambda () (cond ,@clause2))))
			     (if test-result
				 ((thunk2) test-result)
				 (thunk3)))
		    (if (eq? 'else (caadr expr))
			`(begin ,@(cdadr expr))
			`(if ,(caadr expr)
			     (begin ,@(cdadr expr))
			     (cond ,@clause2))))))))))

(define (symbol->string s)
  (string-copy (internal-symbol->string s)))

(define (newline . args)
  (if (null? args)
      (display "\010")
      (display "\010" (car args))))

;;; number? und integer? sind primitives

(define (complex? n) (number? n))
(define (real? n) (number? n))
(define (rational? n) (integer? n))

(define (inexact? n) (and (number? n) (not (exact? n))))

(define (internal-output outfunction argument port)
  (if (null? port)
      (outfunction argument)
      (if (output-port? (car port))
	  (begin
	    (define oldport (current-output-port))
	    (set-current-output-port! (car port))
	    (define rval (outfunction argument))
	    (set-current-output-port! oldport)
	    rval)
	  (error "invalid port / output-procedure"))))

(define (internal-input infunction port)
  (if (null? port)
      (infunction)
      (if (input-port? (car port))
	  (begin
	    (define oldport (current-input-port))
	    (set-current-input-port! (car port))
	    (define rval (infunction))
	    (set-current-input-port! oldport)
	    rval)
	  (error "invalid port / input-procedure"))))

(define (read . port) 
  (internal-input internal-read port))

(define (print expr . port) 
  (internal-output internal-print expr port))

(define (write expr . port) 
  (internal-output internal-write expr port))

(define (print-out expr . port) 
  (internal-output internal-print-out expr port))

(define (display expr . port) 
  (internal-output internal-display expr port))

(define (read-char . port) 
  (internal-input internal-read-char port))

(define (peek-char . port) 
  (internal-input internal-peek-char port))

(define (write-char expr . port) 
  (internal-output internal-write-char expr port))

(define (open-output-file fname)
  (internal-open-output-file fname))

(define (open-extend-file fname)
  (internal-open-output-file-append fname))

(define (open-input-file fname)
  (internal-open-input-file fname))

;;;;

(define (cons* first-element . rest-elements)
  (define (loop this-element rest-elements)
	  (if (null? rest-elements)
	      this-element
	      (cons this-element
		    (loop (car rest-elements)
			  (cdr rest-elements)))))
  (loop first-element rest-elements))

(define (append! . lists)
  (define (loop head tail)
	  (cond ((null? tail) head)
		((null? head) (loop (car tail) (cdr tail)))
		((pair? head)
		 (set-cdr! (last-pair head) (loop (car tail) (cdr tail)))
		 head)
		(else (error "append!: argument not a list"))))
  (if (null? lists)
      '()
      (loop (car lists) (cdr lists))))

(define (reverse! l)
  (if (pair? l)
      (let ((r (reverse l)))
	   (set-car! l (car r))
	   (set-cdr! l (cdr r))
	   r)
      (error "reverse!: argument not a list")))

(define (map* initial-value f . lists)
  (cond ((null? lists)
	 (error "map*: too few arguments"))
	((null? (cdr lists))
	 (let one-loop ((list (car lists)))
	      (if (null? list)
		  initial-value
		  (cons (f (car list))
			(one-loop (cdr list))))))
	(else 
	 (let n-loop ((lists lists))
	      (let parse-cars
		   ((lists lists)
		    (receiver 
		     (lambda (cars cdrs)
			     (cons (apply f cars)
				   (n-loop cdrs)))))
			 (cond ((null? lists)
				(receiver '() '()))
			       ((null? (car lists))
				initial-value)
			       ((pair? (car lists))
				(parse-cars (cdr lists)
					    (lambda (cars cdrs)
						    (receiver 
						     (cons 
						      (car 
						       (car lists)) cars)
						     (cons 
						      (cdr
						       (car lists)) cdrs)))))
			       (else 
				(error "map*: argument not a list"))))))))

(define (last-pair l)
  (if (pair? l)
      (let loop ((l l))
	   (if (pair? (cdr l))
	       (loop (cdr l))
	       l))
      (error "last-pair: argument not a list")))

(define (except-last-pair l)
  (if (pair? l)
      (let loop ((l l))
	   (if (pair? (cdr l))
	       (cons (car l)
		     (loop (cdr l)))
	       '()))
      (error "except-last-pair: argument not a list")))

(define (except-last-pair! l)
  (if (pair? l)
      (if (pair? (cdr l))
	  (begin (let loop ((l l))
		      (if (pair? (cddr l))
			  (loop (cdr l))
			  (set-cdr! l '())))
		 l)
	  '())
      (error "except-last-pair!: argument not a list")))

;;;;
;;;;

(define (call-with-current-continuation procedure)
  (let ((cpair (get-current-continuation)))
    ;;;
    ;;; hier in cpair entweder:  (#t . #<continuation>) 
    ;;;	                 oder:  (<list of functions> . <value>)
    ;;;
    (if (not (eq? (car cpair) #t))  ;; wenn exit-Funktionalitaet
	(begin
	  ;;; cpair enthaelt: ((inits-of-past . exits-of-past) . value)
          (let ((exits-of-past (cdr (car cpair))) ;; 
		(inits-of-future (get-initf))
		(sweep #f))
	     ;;; exits skippen / wenn gleiche inits vorhanden
	     ;;; wenn Unterschied -> Rest der exits aufrufen
             (for-each
	       (lambda (fpair)
		 (if sweep
		     ((cdr fpair))
		     (if (and (pair? inits-of-future)
			      (eq? fpair (car inits-of-future))) ;; ignore ?
			 (set! inits-of-future (cdr inits-of-future))
			 (begin
			   (set! sweep #t)
			   ((cdr fpair)))))) exits-of-past)
	     ;;; Alle inits-of-future (reversed) aufrufen
	     (for-each 
	       (lambda (fpair)
		  ((car fpair)))
		  (reverse inits-of-future))) 
	  (cdr cpair))			;; dann exit mit Wert
	(begin 
          (procedure (cdr cpair))))))	;; sonst call der Prozedur
					;; mit #<continuation>

(define call/cc call-with-current-continuation)

;;; loadname

(define (string-split string char)
   (let ((pos (string-find-next-char string char)))
	(if (string=? string "")
	    '()
  	    (if pos
	        (cons (if (eqv? 0 pos) "" (substring string 0 pos))
		      (string-split 
			(if (eqv? (+ pos 1) (string-length string))
			    ""
                        (substring string (+ pos 1) (string-length string)))
		 	 char))
	        (cons string '())))))

;;; load

(define (internal-load-loop port)
  (define expr (read port))
  (if (eof-object? expr)
      'ok
      (begin
	(eval-in-global-environment expr)
	(internal-load-loop port))))

(define (eload file)
  (define ip)
  (set! ip (open-embedded-input-file file))
  (internal-load-loop ip)
  (close-input-port ip)
)

(define (load . file)
  (define ip)
  (define filename)
  (if (null? file)
      (if (null? the-current-filename)
	  (error "no current filename / load")
	  (begin
	    (load the-current-filename)))
      (begin
	(if (not (string? (car file)))
	    (error "filename not a string"))
	(set! filename (car file))
	(if (eq? (string-ref filename 0) #\\)
	    (begin
	      (set! filename (substring filename 1 (string-length filename)))
	      (set! ip (open-internal-file filename))
	      (if (not ip)
		 (error 
		   (string-append 
		     "file \"" (car file) "\" not found / load (IFS)")))
	      (internal-load-loop ip)
	      (close-input-port ip)
	      (set! the-current-filename filename)
	      'ok)
	    (begin
              (if (not filename)
	           (error 
	             (string-append 
		       "file \"" (car file) "\" not found / load")))
              (if (file-exists? filename)
                  (begin
	            (set! ip (open-input-file filename))
	            (internal-load-loop ip)
	            (close-input-port ip)
	            (set! the-current-filename filename)
	            'ok)
                  (error 
	            (string-append 
		      "file \"" (car file) "\" not found / load"))))))))

;;;
;;; pp - Vorbereitungen
;;;

(define (current-column . port)
  (define op)
  (define rc)
  (if (null? port)
      (+ (internal-current-column) 1)
      (if (output-port? (car port))
        (begin
	  (set! op (current-output-port))
	  (set-current-output-port! (car port))
	  (set! rc (internal-current-column))
	  (set-current-output-port! op)
	  (+ rc 1))
	(error "invalid port / current-column"))))

;;;
;;; (getenv "PATH") ...
;;;

(define (getenv s) (internal-getenv s))

(define (call-with-input-file string proc)
  (let* ((x (open-input-file string))
	 (value (proc x)))
           (close-input-port x)
	   value))

(define (call-with-output-file string proc)
  (let* ((x (open-output-file string))
	 (value (proc x)))
           (close-output-port x)
	   value))

(define (equal? x y)
  (if (eqv? x y)
      #t
      (cond ((pair? x)
	       (if (pair? y)
	           (and (equal? (car x) (car y))
		        (equal? (cdr x) (cdr y)))
	           #f))
	    ((string? x)
	       (if (string? y)
	           (string=? x y)
	           #f))
	    ((vector? x)
	       (if (and (vector? y) (eqv? (vector-length x) (vector-length y)))
	           (equal? (vector->list x) (vector->list y))
	           #f))
	    (else 
	       #f))))
(define / 
    (named-syntax-lambda (/ expr env) 
      (scm-two-arg-expand-left expr '/ 'internal-/ 1)))


(define (atan x . y)
  (if (null? y)
      (internal-atan1 x)
      (internal-atan2 x (car y))))

(define (number->string num . radix)
  (if (null? radix)
      (internal-number->string num 10)
      (internal-number->string num (car radix))))

(define (string->number string . radix)
  (if (null? radix)
      (internal-string->number string 10)
      (if (member (car radix) '(2 8 10 16))
          (internal-string->number string (car radix))
	  (error "invalid radix / string->number"))))

(define internal-gscount 0)
(define (generate-symbol)
  (set! internal-gscount (+ internal-gscount 1))
  (let ((symbolstring (string-append "internal-generated-" 
			  (number->string (getpid))
			  "-"
			  (number->string (rand))
			  "-"
			  (number->string internal-gscount))))
     (if (symbol-exists? symbolstring)
	 (generate-symbol)
	 (string->symbol symbolstring))))

;;; map

(define (internal-map1 proc arg)
  (if (null? arg)
      ()
      (cons (proc (car arg)) (internal-map1 proc (cdr arg)))))

(define (internal-map2 proc arg1 arg2)
  (if (null? arg1)
      ()
      (cons (proc (car arg1) (car arg2))
	    (internal-map2 proc (cdr arg1) (cdr arg2)))))

(define (map proc . args)
  (let ((l (length args)))
    (if (null? args)
	(error "map called with no arguments / map")
	(if (eqv? l 1)
	    (internal-map1 proc (car args))
	    (if (eqv? l 2)
		(internal-map2 proc (car args) (cadr args))
                (if (null? (car args))
                    ()
                    (internal-simple-eval `(cons (proc ,@(internal-map1 
					  (lambda (x) `',(car x)) args)) 
		                 (map proc 
				       ,@(internal-map1 (lambda 
					       (x) `',(cdr x)) args))))))))))
;;; for-each

(define (internal-for-each1 proc arg)
  (if (null? arg)
      ()
      (begin
        (proc (car arg)) 
	(internal-for-each1 proc (cdr arg))
	'ok)))

(define (internal-for-each2 proc arg1 arg2)
  (if (null? arg1)
      ()
      (begin 
	    (proc (car arg1) (car arg2))
	    (internal-for-each2 proc (cdr arg1) (cdr arg2))
	    'ok)))

(define (for-each proc . args)
  (let ((l (length args)))
    (if (null? args)
	(error "for-each called with no arguments / for-each")
	(if (eqv? l 1)
	    (internal-for-each1 proc (car args))
	    (if (eqv? l 2)
		(internal-for-each2 proc (car args) (cadr args))
                (if (null? (car args))
                    'ok
                    (internal-simple-eval `(begin 
				 (proc ,@(internal-map1 
					  (lambda (x) `',(car x)) args)) 
		                 (for-each proc 
				       ,@(internal-map1 (lambda 
					       (x) `',(cdr x)) args))))))))))

;;; do

(define do (named-syntax-lambda (do e env)
  (define loop (generate-symbol))   
  (define varlist (map (lambda (x) (car x)) (cadr e)))
  (define initlist (map (lambda (x) (cadr x)) (cadr e)))
  (define steplist (map (lambda (x) 
		      (if (eqv? (length x) 2)
			  (car x)
			  (caddr x))) (cadr e)))
  (define testexpr (caaddr e))
  (define sequence (cdaddr e))
  (define commands (cdddr e))
  `(letrec ((,loop
	     (lambda ,varlist
		(if ,testexpr
		    (begin ,@sequence)
		    (begin ,@commands
			   (,loop ,@steplist))))))
	   (,loop ,@initlist))))

;;; assoc and friends

(define (internal-assoc proc obj alist)
   (if (null? alist)
       #f
       (if (proc obj (caar alist))
	   (car alist)
	   (internal-assoc proc obj (cdr alist)))))

(define (assq obj alist) (internal-assoc eq? obj alist))
(define (assv obj alist) (internal-assoc eqv? obj alist))
(define (assoc obj alist) (internal-assoc equal? obj alist))

;;; case

(define internal-thunk-count 0)

(define (internal-init-thunk-count)
  (set! internal-thunk-count 0))

(define (internal-get-thunk-symbol)
  (set! internal-thunk-count (+ internal-thunk-count 1))
  (string->symbol (string-append "thunk" 
				 (number->string internal-thunk-count))))

(define (internal-get-thunklist in)
  (if (null? in) 
      '()
      (if (eq? (caar in) 'else)
         `((elsethunk (lambda () ,@(cdar in))))
       (cons `(,(internal-get-thunk-symbol) (lambda () ,@(cdar in)))
	     (internal-get-thunklist (cdr in))))))

(define (internal-get-condlist in)
  (if (null? in)
      '()
      (if (eq? (caar in) 'else)
	  '((else (elsethunk)))
          (cons `((memv key ',(caar in)) (,(internal-get-thunk-symbol)))
	        (internal-get-condlist (cdr in))))))

(define (internal-get-condlist-reset in)
  (internal-init-thunk-count)
  (internal-get-condlist in))

(define (internal-get-thunklist-reset in)
  (internal-init-thunk-count)
  (internal-get-thunklist in))

(define case (named-syntax-lambda (case e env)
            `(let ((key ,(cadr e))
                  ,@(internal-get-thunklist-reset (cddr e)))
		  (cond ,@(internal-get-condlist-reset (cddr e))))))

;;; apply

(define (internal-simple-apply proc arglist)
  (internal-simple-eval 
    (append (list `',proc) (map (lambda (x) `',x) arglist))))

(define (apply proc . args)
  (define (pack l)
    (let  ((a-list (car (reverse l)))
	   (b-list (reverse (cdr (reverse l)))))
	      (append b-list a-list)))
   (internal-simple-apply proc (pack args)))

;;; = > < and friends

(define (= x y . args)
  (if (null? args)
      (internal-= x y)
      (and (internal-= x y)
	   (apply = y (car args) (cdr args)))))

(define (> x y . args)
  (if (null? args)
      (internal-> x y)
      (and (internal-> x y)
	   (apply > y (car args) (cdr args)))))

(define (< x y . args)
  (if (null? args)
      (internal-< x y)
      (and (internal-< x y)
	   (apply < y (car args) (cdr args)))))

(define (<= x y . args)
  (if (null? args)
      (or (internal-< x y) (internal-= x y))
      (and (or (internal-< x y) (internal-= x y))
	   (apply <= y (car args) (cdr args)))))

(define (>= x y . args)
  (if (null? args)
      (or (internal-> x y) (internal-= x y))
      (and (or (internal-> x y) (internal-= x y))
	   (apply >= y (car args) (cdr args)))))

;;; zero and friends

(define (zero? x) (= x 0))
(define (positive? x) (> x 0))
(define (negative? x) (< x 0))

;;; abs

(define (abs x)
  (if (< x 0)
      (* x (- 1))
      x))

;;;

(define (vector-copy v)
  (list->vector (vector->list v)))

(define (modulo x y)
  (let ((r (remainder x y)))
       (if (negative? x)
	   (if (or (negative? y) (zero? r))
	       r
	       (+ y r))
	   (if (and (negative? y) (not (zero? r)))
	       (+ y r)
	       r))))

(define (writeln . args)
  (for-each display args)
  (newline))

(define (even? n)
  (if (integer? n)
      (= (modulo n 2) 0)
      (error "integer expected / even?")))

(define (odd? n)
  (not (even? n)))

(define ++ 
  (named-syntax-lambda (++ expr env)
  `(begin
     (set! ,(cadr expr) (+ ,(cadr expr) 1))
     ,(cadr expr))))

(define -- 
  (named-syntax-lambda (-- expr env)
  `(begin
     (set! ,(cadr expr) (- ,(cadr expr) 1))
     ,(cadr expr))))

;;;
;;; read-until-char und read-line
;;; internal-read-until-char existiert ...
;;;

(define (read-until-char c . port)
  (if (null? port)
      (internal-read-until-char c)
      (if (input-port? (car port))
	  (begin
	    (define oldport (current-input-port))
	    (set-current-input-port! (car port))
	    (define rval (internal-read-until-char c))
	    (set-current-input-port! oldport)
	    rval)
	  (error "invalid port / read-until-char"))))

(define (internal-read-line)
  (let ((s (read-until-char #\newline)))
       (if (not (string? s))
	   s 
	   (begin
	     (if (eqv? (string-length s) 0)
		 (get-eof-object)
		 (begin
		    (let ((last (string-ref s (- (string-length s) 1))))
			 ;;(writeln "LAST= " (string last))
			 (if (eq? last #\newline)
			     (substring s 0 (- (string-length s) 1))
			     (string-append
			       s
			       (let ((x (internal-read-line)))
				    (if (eof-object? x) "" x)))))))))))
			       
(define (read-line . port)
  (internal-input internal-read-line port))

;;;

(define (fak n)
  (if (eqv? n 0)
      1
      (* n (fak (- n 1)))))

;;; min und max

(define (max . args)
   (if (null? args)
       (error "no arguments / max")
       (begin
	  (if (null? (cdr args))
	      (car args)
	      (begin
		 (if (null? (cddr args))   ; zwei Argumente ?
		     (if (>= (car args) (cadr args))
			 (- (+ (car args) (cadr args)) (cadr args))
			 (- (+ (cadr args) (car args)) (car args)))
		     (apply max (max (car args) (cadr args)) (cddr args))))))))

(define (min . args)
   (if (null? args)
       (error "no arguments / min")
       (begin
	  (if (null? (cdr args))
	      (car args)
	      (begin
		 (if (null? (cddr args))   ; zwei Argumente ?
		     (if (<= (car args) (cadr args))
			 (- (+ (car args) (cadr args)) (cadr args))
			 (- (+ (cadr args) (car args)) (car args)))
		     (apply min (min (car args) (cadr args)) (cddr args))))))))

;;; gcd and lcm

(define (gcd . args)
  (define (internal-gcd-2 a b)
     (if (or (< a 0) (< b 0))
	 (internal-gcd-2 (abs a) (abs b))
	 (if 
	   (zero? a)
	   b
	   (if
	     (zero? b)
	     a
	     (if (= a b)
	       (max a b)
	       (internal-gcd-2 (min a b) (- (max a b) (min a b))))))))
  (if (null? args)
      0
      (if (null? (cdr args))
	  (car args)
	  (if (null? (cddr args))
	      (internal-gcd-2 (car args) (cadr args))
	      (apply gcd (internal-gcd-2 (car args) (cadr args))
			 (cddr args))))))

(define (lcm . args)
  (define (internal-lcm-2 a b)
    (let ((x (gcd a b)))
	 (abs (* x (/ a x) (/ b x)))))
  (if (null? args)
      1
      (if (null? (cdr args))
	  (car args)
	  (if (null? (cddr args))
	      (internal-lcm-2 (car args) (cadr args))
	      (apply lcm (internal-lcm-2 (car args) (cadr args))
			 (cddr args))))))

;;; sort and sort!

(define (sort obj pred)
  (if (vector? obj)
      (sort! (vector-copy obj) pred)
      (vector->list (sort! (list->vector obj) pred))))

(define (sort! obj pred)
  (define (exchange! vec i j)
    (let ((x (vector-ref vec i))
	  (y (vector-ref vec j)))
	 (vector-set! vec i y)
	 (vector-set! vec j x)))
  (define (sort-internal! vec l r)
	  (cond
	   ((<= r l)
	    vec)
	   ((= r (+ l 1))
	    (if (pred (vector-ref vec r)
		      (vector-ref vec l))
		(exchange! vec l r)
		vec))
	   (else
	    (quick-merge vec l r))))
  (define (quick-merge vec l r)
	  (let ((first (vector-ref vec l)))
	       (define (increase-i i)
		       (if (or (> i r)
			       (pred first (vector-ref vec i)))
			   i
			   (increase-i (+ i 1))))
	       (define (decrease-j j)
		       (if (or (<= j l)
			       (not (pred first (vector-ref vec j))))
			   j
			   (decrease-j (- j 1))))
	       (define (loop i j)
		       (if (< i j)
			   (begin (exchange! vec i j)
				  (loop (increase-i (+ i 1)) 
					(decrease-j (- j 1))))
			   (begin (if (> j l)
				      (exchange! vec j l))
				  (sort-internal! vec (+ j 1) r)
				  (sort-internal! vec l (- j 1)))))
	       (loop (increase-i (+ l 1))
		     (decrease-j r))))
  (if (vector? obj)
      (begin (sort-internal! obj 0 (- (vector-length obj) 1))
	     obj)
      (error "sort! only works on vectors")))

;;; copy

(define (copy expr)
  (cond ((vector? expr)
           (list->vector (copy (vector->list expr))))
        ((pair? expr)
           (cons (copy (car expr)) (copy (cdr expr))))
        (else expr)))

;;; make-initialized-vector

(define (make-initialized-vector length proc)
  (define v (make-vector length))
  (for ((define index 0)) ((< index length)) ((set! index (+ index 1)))
       (vector-set! v index (proc index)))
  v)

(define (pwd)
  (let* ((ip (popen/r "pwd"))
	 (val (read-line ip)))
	(close-input-port ip)
	val))

;;; dwind and friends

(define (dynamic-wind init-thunk action-thunk exit-thunk)
  (let ((thunk-pair (cons init-thunk exit-thunk)))
    (init-thunk)
    (set-initf! (cons thunk-pair (get-initf)))
    (set-exitf! (cons thunk-pair (get-exitf)))
    (let ((value (action-thunk)))
         (exit-thunk)
	 (set-initf! (cdr (get-initf)))
	 (set-exitf! (cdr (get-exitf)))
         value))) 

;;;

(define fluid-let 
  (named-syntax-lambda (fluid-let expr env)
    (define (internal-variables expr)
	    (if (null? expr) ()
		(cons (caar expr)
		      (internal-variables (cdr expr)))))
    (define (internal-values expr)
        (if (null? expr) ()
            (cons (if (pair? (cdar expr))
                      (cadar expr) 
                      '(get-unassigned-constant))       ;;; unassigned
                  (internal-values (cdr expr)))))
    (define names (internal-variables (cadr expr)))
    (define inits (internal-values (cadr expr)))
    (define body (cddr expr))
    (define (generate-namelist str) 
       (map (lambda (name) 
	  (string->symbol
	    (string-append
	      (symbol->string (generate-symbol))
	      "-"
	      (symbol->string name)
	      "-"
	      str)))
	    names))
    (define new-names (generate-namelist "new"))
    (define old-names (generate-namelist "old"))
    (define (generate-set-expressions to from)
       (map (lambda (to-elem from-elem)
	      `(set! ,to-elem ,from-elem)) to from))
    (define outer-let
       (map (lambda (value init)
	       `(,value ,init))
       new-names
       inits))
    (define inner-let 
       (map (lambda (name) 
	 `(,name ()))
	old-names))
    (define init-sets
       `(,@(generate-set-expressions old-names names)
	 ,@(generate-set-expressions names new-names)))
    (define exit-sets
       `(,@(generate-set-expressions new-names names)
	 ,@(generate-set-expressions names old-names)))
    `(let ,outer-let
       (let ,inner-let
	 (dynamic-wind
	   (lambda ()
	     ,@init-sets)
	   (lambda ()
	     ,@body)
	   (lambda ()
	     ,@exit-sets))))))

(define (complicated-fluid-binding)
  (let ((variable 1)
	(inside-continuation))
    (writeln variable)
    (call/cc
      (lambda (outside-continuation)
	 (fluid-let ((variable 2))
	    (writeln variable)
	    (set! variable 3)
	    (call/cc 
	       (lambda (k)
		  (set! inside-continuation k)
		  (outside-continuation #t)))
	    (writeln variable)
	    (set! inside-continuation #f))))
    (writeln variable)
    (if inside-continuation
	(begin
	  (set! variable 4)
	  (inside-continuation #f)))))

;;; streams

(define (internal-cons-stream a b)
  (make-usertype 'stream (cons a b)))

(define the-empty-stream
  (make-usertype 'stream '()))

(define (stream? obj)
  (and (usertype? obj) 
       (eq? 'stream (get-usertype-type obj))))

(define (internal-car-stream s)
  (if (stream? s)
      (if (empty-stream? s)
	  (error "cannot take head of the empty stream")
          (car (get-usertype-data s)))
      (error "no stream / internal-car-stream")))

(define (internal-cdr-stream s)
  (if (stream? s)
      (if (empty-stream? s)
	  (error "cannot take tail of the empty stream")
          (cdr (get-usertype-data s)))
      (error "no stream / internal-cdr-stream")))

(define (empty-stream? s)
  (if (stream? s)
      (null? (get-usertype-data s))
      (error "no stream / empty-stream?")))

(define (force delayed-object)
  (delayed-object))

;;;

(define make-promise
  (lambda (proc)
    (let ((result-ready? #f)
	  (result #f))
      (lambda ()
	(if result-ready?
	    result
	    (let ((x (proc)))
	      (if result-ready?
		  result
		  (begin
		    (set! result-ready? #t)
		    (set! result x)
		    result))))))))
(define delay 
  (named-syntax-lambda (delay expr env)
	  `(make-promise (lambda () ,@(cdr expr)))))

(define cons-stream 
  (named-syntax-lambda (cons-stream expr env)
	  `(internal-cons-stream ,(cadr expr) (delay ,(caddr expr)))))

(define (head stream)
  (internal-car-stream stream))

(define (tail stream)
  (force (internal-cdr-stream stream)))

(define (nth-stream n s)
  (cond ((empty-stream? s)
	 (error "empty stream - nth-stream"))
	((= n 0)
	 (head s))
	(else 
	 (nth-stream (- n 1) (tail s)))))

(define (stream-map stream proc)
  (if (empty-stream? stream)
      the-empty-stream
      (cons-stream (proc (head stream))
		   (stream-map (tail stream) proc))))

(define (list->stream l)
  (if (null? l)
      the-empty-stream
      (cons-stream (car l) (list->stream (cdr l)))))

(define (stream . l)
  (list->stream l))

(define (stream->list s)
  (if (empty-stream? s)
      '() 
      (cons (head s)
	    (stream->list (tail s)))))

(define stream-pair? stream?)
(define stream-car head)
(define stream-cdr tail)
(define stream-null? empty-stream?)

(define (stream-length s)
  (if (empty-stream? s)
      0
      (+ 1 (stream-length (tail s)))))

(define (stream-ref stream index)
  (nth-stream index stream))

(define (stream-tail stream index)
  (if (= index 0)
      stream
      (stream-tail (tail stream) (- index 1))))

;;;

(define (append-streams sa sb)
  (if (empty-stream? sa)
      sb
      (cons-stream (head sa)
		   (append-streams (tail sa) sb))))

(define (print-stream s)
  (if (empty-stream? s)
      'ok
      (begin (print (head s))
	     (print-stream (tail s)))))

(define append-stream append-streams)

(define (filter-stream s proc)
  (if (empty-stream? s)
      s
      (if (proc (head s))
	  (cons-stream (head s) 
		       (filter-stream (tail s) proc))
	  (filter-stream (tail s) proc))))

;;; for

(define 
  for 
  (named-syntax-lambda (for expr env)
    (define inits (cadr expr))
    (define tests (caddr expr))
    (define steps (cadddr expr))
    (define expressions (cddddr expr))
    (define loopsymbol (generate-symbol))
    `((lambda ()
      ,@inits
      (define ,loopsymbol (lambda ()
	(if (not (and ,@tests))
	    'ok
	    (begin
	       ,@expressions
	       ,@steps
	       (,loopsymbol)))))
	(,loopsymbol)))))

;;; mixed 

(define string->input-port open-input-string)
(define string->port string->input-port)

(define (with-output-to-string thunk)
  (let ((oldport (current-output-port))
        (newport (open-output-string)))
       (dynamic-wind
	  (lambda ()
		  (set-current-output-port! newport))
	  thunk
	  (lambda ()
		  (set-current-output-port! oldport)))
       (get-output-string newport)))

(define (with-input-from-string string thunk)
  (let ((oldport (current-input-port))
        (newport (open-input-string string)))
       (dynamic-wind
	  (lambda ()
		  (set-current-input-port! newport))
	  thunk
	  (lambda ()
		  (set-current-input-port! oldport)))))

(define (with-output-to-file filename thunk)
  (let ((oldport (current-output-port))
        (newport (open-output-file filename)))
       (dynamic-wind
	  (lambda ()
		  (set-current-output-port! newport))
	  thunk
	  (lambda ()
		  (set-current-output-port! oldport)
		  (close-output-port newport)))))

(define (with-input-from-file filename thunk)
  (let ((oldport (current-input-port))
        (newport (open-input-file filename)))
       (dynamic-wind
	  (lambda ()
		  (set-current-input-port! newport))
	  thunk
	  (lambda ()
		  (set-current-input-port! oldport)
		  (close-input-port newport)))))

(define (with-input-from-port port thunk)
  (let ((oldport (current-input-port))
        (newport port))
       (dynamic-wind
	  (lambda ()
		  (set-current-input-port! newport))
	  thunk
	  (lambda ()
		  (set-current-input-port! oldport)))))

(define (with-output-to-port port thunk)
  (let ((oldport (current-output-port))
        (newport port))
       (dynamic-wind
	  (lambda ()
		  (set-current-output-port! newport))
	  thunk
	  (lambda ()
		  (set-current-output-port! oldport)))))

(define (close-port port)
  (if (input-port? port)
      (close-input-port port)
      (if (output-port? port)
	  (close-output-port port)
	  (error "no valid port / close-port"))))

(define close close-port)

(define power expt)
(define ** expt)

(define (truncate x)
  (if (negative? x)
      (ceiling x)
      (floor x)))

(define flush-output-port flush-port)
(define flush flush-port)

(define (exit . args)
  (if (null? args)
      (fixarg-exit 0)
      (fixarg-exit (car args))))

(define (make-string . args)
  (if (null? (cdr args))
      (fixarg-make-string (car args) #\space)
      (fixarg-make-string (car args) (cadr args))))

(define (make-vector . args)
  (if (null? (cdr args))
      (fixarg-make-vector (car args) '())
      (fixarg-make-vector (car args) (cadr args))))

(define (-1+ n) (+ n -1))

;;; pp

(define (char-visible? c)
  (let ((i (char->integer c)))
       (and (> i 32) (< i 127))))

(define (char->decimal-string c)
  (define (cs-helper string)
	  (if (>= (string-length string) 3)
	      string
	      (cs-helper (string-append "0" string))))
  (cs-helper (number->string (char->integer c))))

;;;		Does *not* implement ~| option.
;;;		The ~& option is bogus: it always prints a newline.
;;;
;;;  FUNCTION: (FORMAT <port> <format-string> . <args>)
;;;  
;;;  RESULT: returns unconsumed <args> or a string; has side effect of
;;;  printing according to <format-string>.  If <port> is #t the output is
;;;  to the current input port.  If <port> is #f, a formatted string is
;;;  returned as the result of the call.  Otherwise <port> must be an
;;;  output port.  <format-string> must be a string.  Characters are output
;;;  as if the string were output by the DISPLAY function with the
;;;  exception of those prefixed by a tilde (~) as follows [note that options
;;;  which take arguments remove them from the argument list (they are said to
;;;  be `consumed')]:
;;;
;;;~a  any: display the argument (as for humans).
;;;~s  slashified: write the argument (as for parsers).
;;;~d  decimal: the integer argument is output in decimal format.
;;;~x  hexadecimal: the integer argument is output in hexadecimal format.
;;;~o  octal: the integer argument is output in octal format.
;;;~b  binary: the integer argument is output in binary format.
;;;~p  plural: if the argument is greater than 1, a lower case 's' is printed.
;;;~c  character: the next argument is displayed as a character.
;;;~_  space: output a space character.
;;;~%  newline: output a newline character.
;;;~&  freshline: unless at the beginning of a line, same as ~%, else ignored.
;;;~~  tilde: output a tilde.
;;;~t  tab: output a tab charcter. **implemented, but system dependent**
;;;~g  glorify: pretty print the argument (typically an s-expression).
;;;~?  indirection: take the next argument as a format string and consume
;;;    further arguments as appropriate, then continue to process the current
;;;    format string.
;;;~|  page seperator: output a page seperator.
;;;  

(define tab-character (integer->char 9))   ;; NB: assumes ASCII encoding!!!
(define no-value (string->symbol ""))      ;; the sometimes-non-printing object

(define (format <output-port> <format-string> . <args>)
  (letrec ((port (if (eq? <output-port> #t)
		     (current-output-port)
		     <output-port>))
	   (char-write	   (lambda (any) (write-char any port)))
	   (string-display (lambda (any) (display any port)))
	   (do-write	   (lambda (any) (write   any port)))
	   (do-display	   (lambda (any) (display any port)))
	   (return-value   (lambda () no-value)) ; default
	   )
    (define (format-help format-strg arglyst)
      (letrec
	  (
	   (length-of-format-string (string-length format-strg))
	   (anychar-dispatch
	    (lambda (pos arglist)
	      (if (>= pos length-of-format-string)
		  arglist ; used for ~? continuance
		  (let ( (char (string-ref format-strg pos)) )
		    (cond
		     ((eq? char #\~)
		      (tilde-dispatch (+ pos 1) arglist))
		     (else
		      (char-write char)
		      (anychar-dispatch (+ pos 1) arglist)
		      ))
		    ))
	      )) ; end anychar-dispatch
	    (tilde-dispatch
	     (lambda (pos arglist)
	       (cond
		((>= pos length-of-format-string)
		 (char-write #\~) ; tilde at end of string is just output
		 arglist ; used for ~? continuance
		 )
		(else
		 (case (char-upcase (string-ref format-strg pos))
		   ((#\A)	; Any -- for humans
		    (do-display (special-car arglist))
		    (anychar-dispatch (+ pos 1) (special-cdr arglist))
		    )
		   ((#\S)	; Slashified -- for parsers
		    (do-write (special-car arglist))
		    (anychar-dispatch (+ pos 1) (special-cdr arglist))
		    )
		   ((#\D)	; Decimal
		    (string-display (number->string (special-car arglist) 10))
		    (anychar-dispatch (+ pos 1) (special-cdr arglist))
		    )
		   ((#\X)	; Hexadecimal
		    (string-display (number->string (special-car arglist) 16))
		    (anychar-dispatch (+ pos 1) (special-cdr arglist))
		    )
		   ((#\O)	; Octal
		    (string-display (number->string (special-car arglist)  8))
		    (anychar-dispatch (+ pos 1) (special-cdr arglist))
		    )
		   ((#\B)	; Binary
		    (string-display (number->string (special-car arglist)  2))
		    (anychar-dispatch (+ pos 1) (special-cdr arglist))
		    )
		   ((#\C)	; Character
		    (char-write (special-car arglist)) 
		    (anychar-dispatch (+ pos 1) (special-cdr arglist))
		    )
		   ((#\P)	; Plural
		    (if (= (special-car arglist) 1)
			#f ; no action
			(char-write #\s))
		    (anychar-dispatch (+ pos 1) (special-cdr arglist))
		    )
		   ((#\~)	; Tilde
		    (char-write #\~)
		    (anychar-dispatch (+ pos 1) arglist)
		    )
		   ((#\%)	; Newline
		    (char-write #\newline)
		    (anychar-dispatch (+ pos 1) arglist)
		    )
		   ((#\_)	; Space
		    (char-write #\space)
		    (anychar-dispatch (+ pos 1) arglist)
		    )
		   ((#\&)	; Freshline -- Not Yet Implemented
		    (char-write #\newline)
		    (anychar-dispatch (+ pos 1) arglist)
		    )
		   ((#\T)	; Tab -- Implementation dependent
		    (char-write tab-character) 
		    (anychar-dispatch (+ pos 1) arglist)
		    )
                   ((#\|)	; Page Seperator -- Implementation dependent
                       (char-write #\012) 
                       (anychar-dispatch (+ pos 1) arglist)
                    )
		   ((#\G)	; Pretty-print {T}
		    (if (eq? port #f)
			(string-display (pretty-print-to-string 
					 (special-car arglist)))
			(pretty-print (special-car arglist) port))
		    (anychar-dispatch (+ pos 1) (special-cdr arglist))
		    )
		   ;; {"~?" in Common Lisp is "~K" in T}
		   ((#\?)	; indirection -- take next arg as format string.
		    (anychar-dispatch (+ pos 1) 
				      (format-help (special-car arglist) 
						   (special-cdr arglist)))
		                      ; Note: format-help returns unused args
		    )
		   (else
                    (error "format: unknown tilde escape" 
                           (string-ref format-strg pos)))
		   )))
	       )) ; end tilde-dispatch
	    )			 
	; format-help main 
	(anychar-dispatch 0 arglyst)
	)) ; end format-help
    ; format main
    (if (eq? <output-port> #f)  ;; format to a string
	(let (
	      (out-string-list '())
	      (out-char-list   '())
	     )
	  ;; Grody code!!!  Flush this if we ever get standard string-ports!!
	  (set! do-write
		(lambda (any) (string-display (object->string any))) )
	  (set! do-display
		(lambda (any) (string-display (object->display-string any))))
	  (set! char-write  
		(lambda (c) (set! out-char-list (cons c out-char-list))) )
	  (set! string-display
		(lambda (str)
		  (cond
		   ((null? out-char-list)
		    (set! out-string-list (cons str out-string-list))
		   )
		   (else ;; make chars into string 
		    (set! out-string-list
			  (cons str
				(cons (list->string (reverse out-char-list))
				      out-string-list))
			  )
		    (set! out-char-list '())
		    )))
	   )
	  (set! return-value
		(lambda ()
		  (apply string-append
			 (reverse (if (null? out-char-list)
				      out-string-list
				      (cons (list->string
					     (reverse out-char-list))
					    out-string-list))))))
    )) ;; end-if outport #f
    (format-help <format-string> <args>)
    (return-value)
    ) ; end let
) ; end format

;;; 'generic-write' is a procedure that transforms a Scheme data value (or
;;; Scheme program expression) into its textual representation.  The interface
;;; to the procedure is sufficiently general to easily implement other useful
;;; formatting procedures such as pretty printing, output to a string and
;;; truncated output.
;;;
;;; Parameters:
;;;
;;;   OBJ       Scheme data value to transform.
;;;   DISPLAY?  Boolean, controls whether characters and strings are quoted.
;;;   WIDTH     Extended boolean, selects format:
;;;               #f = single line format
;;;               integer > 0 = pretty-print (value = max nb of chars per line)
;;;   OUTPUT    Procedure of 1 argument of string type, called repeatedly
;;;               with successive substrings of the textual representation.
;;;               This procedure can return #f to stop the transformation.
;;;
;;; The value returned by 'generic-write' is undefined.
;;;
;;; Examples:
;;;
;;;   (write obj)   = (generic-write obj #f #f display-string)
;;;   (display obj) = (generic-write obj #t #f display-string)
;;;
;;; where display-string=(lambda (s) (for-each write-char (string->list s)) #t)

(define (substring-helper s)
  ;; (writeln "SUBSTRING[" s "]")
  (if (= (string-length s) 1)
      (if (char-visible? (special-car (string->list s)))
	 s
	 (char->decimal-string (special-car (string->list s))))
      s))

(define (generic-write obj display? width output)

  (define (read-macro? l)
    (define (length1? l) (and (special-or-pair? l) (null? (special-cdr l))))
    (let ((head (special-car l)) (tail (special-cdr l)))
      (case head
        ((quote quasiquote unquote unquote-splicing) (length1? tail))
        (else                                        #f))))

  (define (read-macro-body l)
    (special-car (special-cdr l)))

  (define (read-macro-prefix l)
    (let ((head (special-car l)) (tail (special-cdr l)))
      (case head
        ((quote)            "'")
        ((quasiquote)       "`")
        ((unquote)          ",")
        ((unquote-splicing) ",@"))))

  (define (out str col)
    (and col (output str) (+ col (string-length str))))

  (define (wr obj col)

    (define (wr-expr expr col)
      (if (read-macro? expr)
        (wr (read-macro-body expr) (out (read-macro-prefix expr) col))
        (wr-lst expr col)))

    (define (wr-lst l col)
      (if (special-or-pair? l)
        (let loop ((l (special-cdr l)) (col (wr (special-car l) (out "(" col))))
          (and col
               (cond ((special-or-pair? l) 
		      (loop (special-cdr l) (wr (special-car l) 
						  (out " " col))))
                     ((null? l) (out ")" col))
                     (else      (out ")" (wr l (out " . " col)))))))
        (out "()" col)))

    (cond ((special-or-pair? obj) (wr-expr obj col))
          ((null? obj)        (wr-lst obj col))
          ((vector? obj)      (wr-lst (vector->list obj) (out "#" col)))
          ((boolean? obj)     (out (if obj "#t" "#f") col))
          ((number? obj)      (out (number->string obj) col))
          ((symbol? obj)      (out (symbol->string obj) col))
          ((primitive-procedure? obj)   
			      (out 
			       (string-append
				  "#<primitive-procedure "
				  (symbol->string (procedure-name obj))
				  ">")
			       col))
          ((compound-procedure? obj)   
			      (out 
			       (if (procedure-name obj)
			          (string-append
				     "#<compound-procedure "
				     (symbol->string (procedure-name obj))
				     ">")
				  "#<compound-procedure>")
			       col))
          ((string? obj)      (if display?
                                (out obj col)
                                (let loop ((i 0) (j 0) (col (out "\"" col)))
                                  (if (and col (< j (string-length obj)))
                                    (let ((c (string-ref obj j)))
                                      (if (or (char=? c #\\)
                                              (char=? c #\"))
                                        (loop j
                                              (+ j 1)
                                              (out "\\"
                                                   (out 
				                     (substring obj i j)
                                                     col)))
                                        (loop i (+ j 1) col)))
                                    (out "\""
                                         (out (substring obj i j) col))))))
          ((char? obj)        (if display?
                                (out (make-string 1 obj) col) ; ok fuer display
                                (out 
				  ;  (case obj
                                  ;    ((#\space)   "space")
                                  ;    ((#\newline) "newline")
                                  ;    (else        (make-string 1 obj)))
				     (cond 
				       ((eqv? obj #\space) "space")
				       ((eqv? obj #\newline) "newline")
				       ((char-visible? obj)
					      (make-string 1 obj))
				       (else (char->decimal-string obj)))
                                     (out "#\\" col))))
          ((input-port? obj)  (out "#<input-port>" col))
          ((output-port? obj) (out "#<output-port>" col))
          ((eof-object? obj)  (out "#<eof-object>" col))
	  ((usertype? obj)    (out 
			       (string-append
				"#<"
				(symbol->string (get-usertype-type obj))
				">")
			       col))
          (else               (out "#<unknown>" col))))

  (define (pp obj col)

    (define (spaces n col)
      (if (> n 0)
        (if (> n 7)
          (spaces (- n 8) (out "        " col))
          (out (substring "        " 0 n) col))
        col))

    (define (indent to col)
      (and col
           (if (< to col)
             (and (out (make-string 1 #\newline) col) (spaces to 0))
             (spaces (- to col) col))))

    (define (pr obj col extra pp-pair)
      (if (or (special-or-pair? obj) 
	      (vector? obj)) ; may have to split on multiple lines
        (let ((result '())
              (left (min (+ (- (- width col) extra) 1) max-expr-width)))
          (generic-write obj display? #f
            (lambda (str)
              (set! result (cons str result))
              (set! left (- left (string-length str)))
              (> left 0)))
          (if (> left 0) ; all can be printed on one line
            (out (reverse-string-append result) col)
            (if (special-or-pair? obj)
              (pp-pair obj col extra)
              (pp-list (vector->list obj) (out "#" col) extra pp-expr))))
        (wr obj col)))

    (define (pp-expr expr col extra)
      (if (read-macro? expr)
        (pr (read-macro-body expr)
            (out (read-macro-prefix expr) col)
            extra
            pp-expr)
        (let ((head (special-car expr)))
          (if (symbol? head)
            (let ((proc (style head)))
              (if proc
                (proc expr col extra)
                (if (> (string-length (symbol->string head))
                       max-call-head-width)
                  (pp-general expr col extra #f #f #f pp-expr)
                  (pp-call expr col extra pp-expr))))
            (pp-list expr col extra pp-expr)))))

    ; (head item1
    ;       item2
    ;       item3)
    (define (pp-call expr col extra pp-item)
      (let ((col* (wr (special-car expr) (out "(" col))))
        (and col
             (pp-down (special-cdr expr) col* (+ col* 1) extra pp-item))))

    ; (item1
    ;  item2
    ;  item3)
    (define (pp-list l col extra pp-item)
      (let ((col (out "(" col)))
        (pp-down l col col extra pp-item)))

    (define (pp-down l col1 col2 extra pp-item)
      (let loop ((l l) (col col1))
        (and col
             (cond ((special-or-pair? l)
                    (let ((rest (special-cdr l)))
                      (let ((extra (if (null? rest) (+ extra 1) 0)))
                        (loop rest
                              (pr (special-car l) 
				  (indent col2 col) extra pp-item)))))
                   ((null? l)
                    (out ")" col))
                   (else
                    (out ")"
                         (pr l
                             (indent col2 (out "." (indent col2 col)))
                             (+ extra 1)
                             pp-item)))))))

    (define (pp-general expr col extra named? pp-1 pp-2 pp-3)

      (define (tail1 rest col1 col2 col3)
        (if (and pp-1 (special-or-pair? rest))
          (let* ((val1 (special-car rest))
                 (rest (special-cdr rest))
                 (extra (if (null? rest) (+ extra 1) 0)))
            (tail2 rest col1 (pr val1 (indent col3 col2) extra pp-1) col3))
          (tail2 rest col1 col2 col3)))

      (define (tail2 rest col1 col2 col3)
        (if (and pp-2 (special-or-pair? rest))
          (let* ((val1 (special-car rest))
                 (rest (special-cdr rest))
                 (extra (if (null? rest) (+ extra 1) 0)))
            (tail3 rest col1 (pr val1 (indent col3 col2) extra pp-2)))
          (tail3 rest col1 col2)))

      (define (tail3 rest col1 col2)
        (pp-down rest col2 col1 extra pp-3))

      (let* ((head (special-car expr))
             (rest (special-cdr expr))
             (col* (wr head (out "(" col))))
        (if (and named? (special-or-pair? rest))
          (let* ((name (special-car rest))
                 (rest (special-cdr rest))
                 (col** (wr name (out " " col*))))
            (tail1 rest (+ col indent-general) col** (+ col** 1)))
          (tail1 rest (+ col indent-general) col* (+ col* 1)))))

    (define (pp-expr-list l col extra)
      (pp-list l col extra pp-expr))

    (define (pp-lambda expr col extra)
      (pp-general expr col extra #f pp-expr-list #f pp-expr))

    (define (pp-if expr col extra)
      (pp-general expr col extra #f pp-expr #f pp-expr))

    (define (pp-cond expr col extra)
      (pp-call expr col extra pp-expr-list))

    (define (pp-case expr col extra)
      (pp-general expr col extra #f pp-expr #f pp-expr-list))

    (define (pp-and expr col extra)
      (pp-call expr col extra pp-expr))

    (define (pp-let expr col extra)
      (let* ((rest (special-cdr expr))
             (named? (and (special-or-pair? rest) 
			  (symbol? (special-car rest)))))
        (pp-general expr col extra named? pp-expr-list #f pp-expr)))

    (define (pp-begin expr col extra)
      (pp-general expr col extra #f #f #f pp-expr))

    (define (pp-do expr col extra)
      (pp-general expr col extra #f pp-expr-list pp-expr-list pp-expr))

    ; define formatting style (change these to suit your style)

    (define indent-general 2)

    (define max-call-head-width 5)

    (define max-expr-width 50)

    (define (style head)
      (case head
        ((lambda let* letrec define) pp-lambda)
        ((if set!)                   pp-if)
        ((cond)                      pp-cond)
        ((case)                      pp-case)
        ((and or)                    pp-and)
        ((let)                       pp-let)
        ((begin)                     pp-begin)
        ((do)                        pp-do)
        (else                        #f)))

    (pr obj col 0 pp-expr))

  (if width
    (out (make-string 1 #\newline) (pp obj 0))
    (wr obj 0)))

;;; (reverse-string-append l) = (apply string-append (reverse l))

(define (reverse-string-append l)
  (define (rev-string-append l i)
    (if (special-or-pair? l)
      (let* ((str (special-car l))
             (len (string-length str))
             (result (rev-string-append (special-cdr l) (+ i len))))
        (let loop ((j 0) (k (- (- (string-length result) i) len)))
          (if (< j len)
            (begin
              (string-set! result k (string-ref str j))
              (loop (+ j 1) (+ k 1)))
            result)))
      (make-string i)))
  (rev-string-append l 0))

;;; (object->string obj) returns the textual representation of 'obj' as a
;;; string.
;;;
;;; Note: (write obj) = (display (object->string obj))

(define (object->string obj)
  (let ((result '()))
    (generic-write obj #f #f (lambda (str) (set! result (cons str result)) #t))
    (reverse-string-append result)))

;;; Added object->display-string for format - KenD

(define (object->display-string obj)
  (let ((result '()))
    (generic-write obj #t #f (lambda (str) (set! result (cons str result)) #t))
    (reverse-string-append result)))

;;; (object->limited-string obj limit) returns a string containing the first
;;; 'limit' characters of the textual representation of 'obj'.

(define (object->limited-string obj limit)
  (let ((result '()) (left limit))
    (generic-write obj #f #f
      (lambda (str)
        (let ((len (string-length str)))
          (if (> len left)
            (begin
              (set! result (cons (substring str 0 left) result))
              (set! left 0)
              #f)
            (begin
              (set! result (cons str result))
              (set! left (- left len))
              #t)))))
    (reverse-string-append result)))

;;; (pretty-print obj port) pretty prints 'obj' on 'port'.  The current
;;; output port is used if 'port' is not specified.

(define (pretty-print obj . opt)
  (let ((port (if (special-or-pair? opt) 
		  (special-car opt) (current-output-port))))
       (if (compound-procedure? obj)
	   (set! obj
		 (cons 'lambda 
		       (cons (procedure-parameters obj)
			     (procedure-body obj)))))
       (if (syntax-procedure? obj)
	   (set! obj
	     `(named-syntax-lambda 
		(,(procedure-name obj) 
		 ,@(procedure-parameters obj))
		,@(procedure-body obj))))
       (generic-write obj #f 79 (lambda (s) (display s port) #t))
       'ok))

;;; (pretty-print-to-string obj) returns a string with the pretty-printed
;;; textual representation of 'obj'.

(define (pretty-print-to-string obj)
  (let ((result '()))
    (generic-write obj #f 79 (lambda (str) (set! result (cons str result)) #t))
    (reverse-string-append result)))

(define pp pretty-print)

;;; eval

(define eval (named-syntax-lambda (eval e env)
  (define expression (cadr e))
  (define environment (cddr e))
  (if (not (null? environment))
      `(begin
	 (internal-set-expr-env-pair! (cons ,expression ,(car environment)))
	 (internal-general-eval))
      `(internal-simple-eval ,expression))))

;;; list? nach IEEE auch bei zirkulaeren Strukturen #f !

(define (list? obj)
  (define already-visited '())
  (define (listproc obj) 
    (if 
      (eq? obj '())
      #t
      (begin
	(if 
	  (not (pair? obj))
	  #f
	  (if 
	    (memq obj already-visited)
	    #f
	    (begin
	      (set! already-visited (cons obj already-visited))
	      (listproc (cdr obj)) ) ) ) ) ) )
  (listproc obj))

;;; listref hat offenbar gefehlt !

(define (list-ref l index)
  (vector-ref (list->vector l) index))

;;; takamichi

(define (tak x y z)
   (if (not (< y x))
       z
       (tak (tak (1- x) y z)
            (tak (1- y) z x)
    	    (tak (1- z) x y))))

(define (perf)
  (set-destructive-mode! #t)
  (gc)
  (let ((x (get-time)))
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (tak 18 12 6)
       (/ (- (get-time) x) 20)))

(define (perf-info)
   (writeln " (tak 18 12 6) => 7 ")
   (writeln " (perf) =>")
   (writeln "                 4.4   Pentium 90 FreeBSD 2.0.5 2MB total memory")
   (writeln "                 3.8   Pentium 90 FreeBSD 2.1.0 4MB total memory")
   (writeln "                 3.7   Pentium 90 FreeBSD 2.1.5 4MB total memory")
   (writeln "                 2.2   Alpha-Server 16MB total memory, full optimized -DNDEBUG")
   (writeln "                 1.0   Pentium-Pro 200, FreeBSD 2.1.5 4MB memory")
   (writeln "                 0.9   Pentium-Pro 200, FreeBSD 2.1.5 16MB memory")
)

;;;

(define (stat)
  (gc)
  (writeln "created: " (creation-date) " " (creation-time))
  (writeln "destructive macro expansion is " (if (get-destructive-mode) "ON" "OFF") ".")
  (writeln "running on a " (* (internal-wordsize) 8) "-bit machine.")
  (writeln
    "Heap   : "
    (/ (* (heap-size) (internal-wordsize) 2) (* 1024 1024))
    " MB, "
    (heap-used) " DWords of " (heap-size) " used, "
    (* (/ (- (heap-size) (heap-used)) (heap-size)) 100) " % free.")
  (writeln
    "AuxMem : "
    (auxmem-size)
    " bytes of "
    (auxmem-maxsize) " used, "
    (* (/ (- (auxmem-maxsize) (auxmem-size)) (auxmem-maxsize)) 100) " % free.")
  (writeln
    "Symbols: "
    (- (get-total-symbols) (get-free-symbols))
    " Symbols of "
    (get-total-symbols)
    " used, "
    (get-free-symbols)
    " available.")
  (writeln
    "Hash   : "
    (get-hashtable-used)
    " entries of "
    (get-hashtable-size)
    " used, "
    (- (get-hashtable-size) (get-hashtable-used))
    " free, max. length of bucket-list: "
    (get-hashtable-max))
  (writeln
    "Stacks : "
    (get-pstack-ptr) "/" (get-pstack-size) " (pstack) "
    (get-cstack-ptr) "/" (get-cstack-size) " (cstack) " ))

(define (get-output-string string-port)
  (list->string (reverse (get-output-charlist string-port))))

;;; slib-Kompatibilitaet

(define (argv)
  (vector->list (argument-vector)))

;;; Listen-Transformationen nach MIT-Scheme

(define (list-transform-positive l p)
  (let ((r '()))
    (for-each (lambda (x) (if (p x) (set! r (cons x r)))) l)
    (reverse r)))

(define (list-transform-negative l p)
  (let ((r '()))
    (for-each (lambda (x) (if (not (p x)) (set! r (cons x r)))) l)
    (reverse r)))

(define (reverse l)
  (define (worker dest source)
    (if (null? source)
	dest
	(worker (cons (car source) dest) (cdr source))))
  (worker '() l))

;;; ------ START OF BITMAP EXTENSIONS -----------------------------------------

(define (v x)
  (define tf "/tmp/bla.tiff")
  (bitmap-writetiff x tf)
  (system (string-append "atiff " tf)))

(define (e x)
  (define tf "/tmp/xx.xbm")
  (bitmap-writexbm x tf)
  (system (string-append "bitmap " tf)))

(define (bitmap-vglue a b)
  (let ((aa (bitmap-copy a))
	(bb (bitmap-copy b)))
    (let ((r (+ (bitmap-height aa) (bitmap-height bb)))
	  (c (max (bitmap-width aa) (bitmap-width bb))))
      (let ((nb (bitmap-create r c)))
	(bitmap-orow-set! aa 0)
	(bitmap-ocol-set! aa 0)
	(bitmap-orow-set! bb (bitmap-height a))
	(bitmap-ocol-set! bb 0)
	(bitmap-implant! aa nb)
	(bitmap-implant! bb nb)
	nb ))))

;;;

(define (bitmap-explode bitmap)
  (define (looper bm row column collector)
    (let ((x (bitmap-nextblack bm row column)))
      (let ((b (bitmap-extract! bm (car x) (cdr x))))
	(if (not b)
	  collector
	  (looper bm (car x) (cdr x) (cons b collector))))))
  (looper (bitmap-copy bitmap) 0 0 '()))   

;;;

(define (bitmap-size bm)
  (* (bitmap-width bm) (bitmap-height bm)))

(define (go)
  (bitmap-readxbm "IMAGES/test.xbm"))

(define (bitmap-equality a b)
  (let ((x (bitmap-compare a b)))
    (/ (car x) (cdr x))))

(define (bitmap-implode list rows cols)
  (let ((ziel (bitmap-create rows cols)))
    (for-each (lambda (x) (bitmap-implant! x ziel)) list)
    ziel))

(define (bitmap-invert bitmap)
  (bitmap-invert! (bitmap-copy bitmap)))

(define (bitmap-tiffpage processor infile . outfile)
  (define (scaleloop input output page)
    (define bm (bitmap-readmtiff input page))
    (if (not bm)
	'ok
	(begin
	  (let ((hbm (processor page bm)))
	    (bitmap-appendtiff hbm output)
	    (scaleloop input output (+ page 1))))))
  (define (procloop input page)
    (define bm (bitmap-readmtiff input page))
    (if (not bm)
	'ok
	(begin
	  (let ((hbm (processor page bm)))
	    (procloop input (+ page 1))))))
  (if (null? outfile)
    (procloop infile 0)
    (scaleloop infile (car outfile) 0)))

(define (bitmap-tifffile->list filename)
  (define l '())
  (define (proc page bitmap)
     (set! l (cons bitmap l)))
  (bitmap-tiffpage proc filename)
  (reverse l))

(define (bitmap-list->tifffile l filename)
  (define (proc l filename)
    (if (null? l)
	'ok
	(begin
	  (bitmap-appendtiff (car l) filename)
	  (proc (cdr l) filename))))
  (if (not (null? l))
    (begin
      (bitmap-writetiff (car l) filename)
      (proc (cdr l) filename))))

(define (greymap-smooth greymap)
  (greymap-smooth-region greymap 0 0 (greymap-height greymap) (greymap-width greymap) 3))

(define (greymap-smooth/v greymap value)
  (greymap-smooth-region greymap 0 0 (greymap-height greymap) (greymap-width greymap) value))

(define (st33-to-tiff sourcelist dest)
  (define n (length sourcelist))
  (define names (make-string 0))
  (for-each 
    (lambda (s)
      ;; (writeln s)
      (set! names (string-append names s))
      (set! names (string-append names "\000")))
    sourcelist)
  (internal-st33-to-tiff n names dest))

;;; ------ END OF BITMAP EXTENSIONS -------------------------------------------

(define #!/usr/home/lisp/SCHEME/scheme)
(define #!/home/lisp/SCHEME/scheme)
(define #!/usr/bin/scheme)
(define #!/bin/scheme)
(define #!/sbin/scheme)
(define #!/usr/sbin/scheme)
(define #!/opt/bin/scheme)
(define #!/usr/local/scheme)
(define #!/usr/local/bin/scheme)

(let ((home (getenv "HOME")))
  (if home
    (let ((file (string-append home "/.schemerc")))
      (if (file-exists? file)
	(load file)))))

(define (the-very-first-top-level-loop)
  (define the-input-port
     (if (> (vector-length (argument-vector)) 1)
         (string->port (vector-ref (argument-vector) 1))  
         (string->port "")))
  (define (internal-loop ip)
    (let ((expr (read ip)))
      (if (eof-object? expr)
	  'ok
	  (if (symbol? expr)
	      (begin
		(with-input-from-port
		  (console-input-port)
		  (lambda ()
	            (load (symbol->string expr))))
		(exit 0) ; exit nach dem load eines files 
	        'ok)
	      (begin
	        (with-input-from-port
	          (console-input-port)
	          (lambda ()
	             (eval-in-global-environment expr)))
		(exit 0) ; exit nach dem eval einer expr   
	        'ok))
	    )))
  (internal-loop the-input-port))

;;; Wenn ein Symbol in argv[1]: load file
;;; Wenn eine Expr in argc[1]:  eval expr

(the-very-first-top-level-loop)

(define (info)
  (display "
  This is Inlab-Scheme ")
  (writeln (cadr (string-split "$Revision: 4.108 $" #\space)))
  (display
  " 
  Copyright (c) 1991-2005,2006 by Inlab Software GmbH, Gruenwald, Germany
  and Thomas Obermair ( obermair@acm.org )
  All Rights Reserved / Alle Rechte vorbehalten.

  This software is licensed under the GPL version 2, see the file
  COPYING for details.

  THIS SOFTWARE IS PROVIDED BY INLAB SOFTWARE GMBH ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INLAB SOFTWARE
  GMBH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
  OF SUCH DAMAGE.

  "))

;;; ------- REP-LOOP

(define (high-level-error-handler
	   env expr msg irr chain jumpin cmsg)
  (writeln "WARNING: high-level-error-handler CALLED TOO EARLY, internal error ? ")  
  (writeln "env:" env)
  (writeln "expr:" expr)
  (writeln "msg:" msg)
  (writeln "irr:" irr)
  (writeln "chain:" chain)
  (writeln "jumpin:" jumpin)
  (writeln "cmsg:" cmsg)
  (exit 2))

(define (call-with-current-unprotected-continuation procedure)
  (let ((cpair (get-current-continuation)))
     (if (not (eq? (car cpair) #t))
	 (begin
	   (let ((exits-of-past (cdr (car cpair)))
		 (inits-of-past (car (car cpair))))
	      (if (not (null? (get-exitf))) 
		  (begin
		   (display 
		    "WARNING: exits lost in call/cuc" (current-error-port))
		    (newline (current-error-port))))
	      (if (not (null? (get-initf)))
		  (begin
		   (display 
		    "WARNING: inits lost in call/cuc" (current-error-port))
		    (newline (current-error-port))))
	      (set-initf! inits-of-past)	;; Vererbung ...
	      (set-exitf! exits-of-past))
	   (cdr cpair)) 			;; jump-in
	 (procedure (cdr cpair)))))		;; call

(define call/cuc call-with-current-unprotected-continuation)

;;;
;;; disable/enable-interrupt wird wieder definiert, weil leichtfertig 
;;; umbenannt ... das ganze error-handling war kaputt !

(define disable-interrupts disable-interrupt)
(define enable-interrupts enable-interrupt)

;;;
;;; Geruest fuer continuation-Aktivierung von eval:
;;;

(call/cuc 
  (lambda (return)
    (call/cuc 
      (lambda (entry)
	(error-handler-set!-jumpout entry)
	(return 'ok)))
        (low-level-error-handler)
    (reset)))

;;; Low-Level-Error-Handler: Importieren der gewuenschten Register 
;;; und Aktivierung des eigentlichen Error-Handlers.

(define (low-level-error-handler)

  ;; (display "*low-level-error-handler*" (console-error-port))
  ;; (display #\newline (console-error-port))

  (disable-interrupts)	;;; Kritisch ...
  (let ((error-environment (error-handler-get-error-environment))
	(error-expression  (error-handler-get-error-expression))
	(error-message     (error-handler-get-error-message))
	(error-irritant    (error-handler-get-error-irritant))
	(error-callchain   (error-handler-get-error-callchain))
	(error-jumpin	   (error-handler-get-jumpin))
	(error-cmessage    (error-handler-get-continue-message)))
    (clean-relevant-error-registers)
    (enable-interrupts)
    (high-level-error-handler 
      error-environment
      error-expression
      error-message
      error-irritant
      error-callchain
      error-jumpin
      error-cmessage)))

;;; Display Errormessage 

(define (display-initial-error-message
	   env expr msg irr chain jumpin cmsg)
    (let ((op (console-output-port)))
	  ;;; (newline op)
	  ;;; (display " ************ ERROR-HANDLER ************")
	  ;;; (newline op) (newline op)
	  ;;; (display " env    : " op) (print env op)
	  ;;; (display " expr   : " op) (print expr op)
	  ;;; (display " msg    : " op) (print msg op)
	  ;;; (display " irr    : " op) (print irr op)
	  ;;; (display " chain  : " op) (print chain op)
	  ;;; (display " jumpin : " op) (print jumpin op)
	  ;;; (display " cmsg   : " op) (print cmsg op)

	  (newline op)
	  (display "ERROR!" op) (newline op)
	  (display "  message   : " op) (display msg op) (newline op)
	  (display "  irritant  : " op) (display irr op) (newline op)
	  (display "  expression: " op) (display expr op) (newline op)
	  (if (continuation? jumpin)
	      (begin
	        (display "  continue  : possible" op) 
		(if (string? cmsg)
		    (begin
		     (display ", " op) (display cmsg op) (newline op))
		    (newline)))
	      (begin
	        (display "  continue  : not possible" op) (newline op)))

	  ;;; (display " pstack : " op) (print (get-pstack jumpin) op)
	  ))

;;;; ------------ Start des eigentlichen Error-Handlings ------------

(define error-level 0)

(define (high-level-error-handler 
	   env expr msg irr chain jumpin cmsg)
  ;;; (display "*high-level-error-handler*" (console-error-port))
  ;;; (display #\newline (console-error-port))
  (define saved-env env)
  (define ip (console-input-port))
  (define op (console-error-port))
  (define (on-screen . args)
     (for-each 
	(lambda (s)
		(display s op)
		(newline op))
      args))
  (define (error-rep)
     (define rexpr)
     (newline op)
     (display "Error (? for help) >> " op)
     (set! rexpr (read ip))

     (if (eof-object? rexpr)
         (begin 
           (clearerr-port (console-input-port))
	   (reset)))
     (case rexpr 
	((? h)
	  (on-screen 
	    ""
	    "  ? h   this help-screen" 
	    "  ee    eval in error-environment"
	    "  eg    eval in global-environment"
	    "  ce    continue with value (evaluated in error-environment)"
	    "  cg    continue with value (evaluated in global-environment)"
	    "  i     continue and return '()" 
	    "  d     (re-) display initial error-message"
	    "  fd    display current environment-frame"
	    "  fu    move one environment-frame up to the caller"
	    "  fi    restore initial environment-frame"
	    "  r     (or EOF) reset to top-level"
	    "  x     exit immediately"))
	((r) 
	  (reset))
	((ee)
	 (newline op)
	 (display "enter expression: " op)
	 (let ((expr (read ip)))
	      (display "value returned  : " op)
	      (print (eval expr env))))
	((eg)
	 (newline op)
	 (display "enter expression: " op)
	 (let ((expr (read ip)))
	      (display "value returned  : " op)
	      (print (eval expr (get-global-environment)))))
	((ce)
	 (if (continuation? jumpin)
	     (begin
	      (newline op)
	      (display "enter value to be returned: " op)
	      (jumpin 
	         (eval (read ip) env)))
	     (begin
	      (display "cannot continue from error ...." op)
	      (newline op))))
	((i)
	 (if (continuation? jumpin)
	     (begin
	      (jumpin '())) 
	     (begin
	      (display "cannot continue from error ...." op)
	      (newline op))))
	((cg)
	 (if (continuation? jumpin)
	     (begin
	      (display "enter value to be returned: " op)
	      (jumpin (eval (read ip) (get-global-environment))))
	     (begin
	      (display "cannot continue from error ...." op)
	      (newline op))))
	((d)
         (display-initial-error-message 
	   env expr msg irr chain jumpin cmsg))
	((fd)
	 (begin     
	    (define (denv e)
		    (if (or (not (environment? e)) 
			    (null? (environment-variable e)))
			#t
			(begin
			  (writeln (environment-variable e) "="
				   (environment-value e))
			  (denv (environment-next e)))))
	    (define (frame-empty? frame)
		    (if (environment? frame)
			(if (environment? (environment-next frame))
			    (if (null? (environment-variable
					 (environment-next frame)))
				#t
				#f)
			    (begin
			      ;; (writeln "WARNING: invalid environment ...")
			      #t))
			(begin 
			 ;; (writeln "WARNING: invalid environment ...")
			 #t)))
	    (if (frame-empty? env)
		(writeln "**** EMPTY FRAME ****")
	        (denv (environment-next env)))))
	((fu)
	 (define (search-next-frame env)
		 (if (not (environment? env))
		     #f
		     (if (null? (environment-variable env))
			 env
			 (search-next-frame (environment-next env)))))
	 (let ((newenv (search-next-frame (environment-next env))))
	      (if newenv
		  (begin
		    (set! env newenv)
		    (writeln "moved up ..."))
		  (writeln "current frame has no other parent ..."))))
	((fi)
	 (set! env saved-env)
	 (writeln "original frame restored ..."))
	((x)
	 (exit 2))
	(else 
	  (newline op)
	  (display "invalid input (? for help)" op)
	  (newline op)))
	  (error-rep))
  (display-initial-error-message 
	   env expr msg irr chain jumpin cmsg)
  (error-rep))

;;; --- Error-Protection
;;;     args fuer receiver:  env expr msg irr chain jumpin cmsg)
;;;	ersetzt werden muss high-level-error-handler

(define (error-protect thunk receiver)
  (define (internal-error-protect thunk)
    (call/cc 
       (lambda (exit)
	  (define (error-substitute . args)
	     (exit (cons '#f args)))
	  (fluid-let 
	     ((high-level-error-handler error-substitute))
	     (cons #t (thunk))))))
  (let ((rc (internal-error-protect thunk)))
       (if (car rc)
	   (cdr rc)
	   (apply receiver (cdr rc)))))

	    
(define top-level-input-line 0)
(define top-level-continuation)

(define (list-ref l n)
  (if (null? l)
      '()
      (if (eqv? n 0)
          (car l)
          (list-ref (cdr l) (- n 1)))))

(define (list-tail l n)
  (if (null? l)
      '()
      (if (eqv? n 0)
          l
          (list-tail (cdr l) (- n 1)))))

(define temporary-directory "/var/tmp")
(define temporary-number 1)

(define (make-temporary-filename)
  (let ((x (getpid)) (y temporary-number))
    (set! temporary-number (+ y 1))
    (string-append temporary-directory
		   "/scm"
		   (number->string y)
		   "."
		   (number->string (getpid)))))

(define top-level-expressions '())
(define top-level-history-depth 20)

(define (set-history-depth! n)
  (set! top-level-history-depth n)
  (if (> (length top-level-expressions) 
	 top-level-history-depth)
      (set-cdr! (list-tail top-level-expressions 
	        top-level-history-depth) '()))
  'ok)

(define (get-top-level-expression n)
  (define ref (- top-level-input-line n))
  (if (or (negative? n) (zero? n))
      (error "invalid top-level-reference" n)
      (if (or (negative? ref) (zero? ref) (> ref top-level-history-depth))
        (error "invalid top-level-reference" n)
        (list-ref top-level-expressions ref))))

(define (! . n)
  (define ref (- top-level-input-line 1))
  (if (not (null? n))
      (set! ref (car n)))
  (letrec ((filename (make-temporary-filename))
	   (tlexpr (get-top-level-expression ref))
	   (oport (open-output-file filename)))
    (pp tlexpr oport)
    (close-output-port oport)
    (let ((sve the-current-filename))
	 (edit filename)
	 (set! the-current-filename sve))
    (call-with-input-file filename 
	(lambda (iport)
	  (let ((xx (read iport)))
		 (if (eof-object? xx)
		     (string->symbol "empty file, nothing evaluated")
		     (begin
	               (set-car! top-level-expressions xx)
	               (eval-in-global-environment xx))))))
	  (delete-file filename)))

(define (!! . n)
  (define ref (- top-level-input-line 1))
  (if (not (null? n))
      (set! ref (car n)))
  (let ((expr (get-top-level-expression ref)))
	   (set-car! top-level-expressions expr)
           (eval-in-global-environment expr)))

(define (?)
  (newline)
  (define (ph line lst)
	  (if (null? lst)
	      'ok
	      (begin
	         (display "   [") (display line) (display "]: ")
		 (print (car lst))
		 (ph (+ line 1) (cdr lst)))))
  (if (< top-level-history-depth (length top-level-expressions))
      (ph (+ (- top-level-input-line (length top-level-expressions)) 2)
          (cdr (reverse top-level-expressions)))
      (ph (+ (- top-level-input-line (length top-level-expressions)) 1)
          (reverse top-level-expressions))))

(define (top-level-rep-loop)
  (define expression)
  (define (display-top-level-prompt)
    (display "[")
    (display top-level-input-line)
    (display "] "))
  (call/cc
    (lambda (cnt)
      (set! top-level-continuation cnt)
      (set! top-level-input-line (+ 1 top-level-input-line))
      (display-top-level-prompt)
      (set! expression (read (console-input-port)))
      (if (and (eof-object? expression)
               (begin
                 (clearerr-port (console-input-port))
 		 (newline)
		 (display "type EOF again to exit ...")
		 (newline)
		 (display-top-level-prompt)
       		 (set! expression (read (console-input-port)))
                 (if (eof-object? expression)
                     #t #f)))
	  (begin
	    (newline)
            (exit 0))
          (begin
	     (set! top-level-expressions 
		   (cons expression top-level-expressions))
	     (if (> (length top-level-expressions) 
		    top-level-history-depth)
		 (set-cdr! (list-tail top-level-expressions 
				      top-level-history-depth) '()))
             (print (eval-in-global-environment expression))))))
  (top-level-rep-loop))

;;; reset / sofort zu Top-Level 

(define (reset)
  (if (procedure? top-level-continuation)
      (begin
	(newline)
        (top-level-continuation '()))
      (error "no top-level-continuation active")))

;;;

(newline)
(display "Inlab-Scheme ")
(writeln (cadr (string-split "$Revision: 4.108 $" #\space)))
(writeln "Copyright (c) 1991-2006,2006 by Inlab Software GmbH, Gruenwald, Germany")
(writeln "All rights reserved, type (info) for details.")
(writeln)

;;;

(set-exit-on-error! #f)
(top-level-rep-loop)

