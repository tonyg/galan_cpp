<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY xform-classes.dsl SYSTEM "xform-classes.dsl">
]>

<style-sheet>
<style-specification id="galandoc">
<style-specification-body>

&xform-classes.dsl;

;---------------------------------------------------------------------------

(define %base-filename%
  "webdoc-output")

(define %chunking-depth%
  1)

(define (section-path #!optional (n (current-node)))
  (let ((part-path (hierarchical-number-recursive (normalize "section") n)))
    (if (equal? (gi n) (normalize "section"))
	(append part-path (list (child-number n)))
	part-path)))

(define (section-path-string #!optional (n (current-node)))
  (string-join "." (section-path n)))

(define (base-filename)
  (if #f
      (walk-root '("fileprefix"))
      %base-filename%))

(define (construct-filename special)
  (string-append (base-filename)
		 (if (and special (not (string=? special "")))
		     (string-append "-" special)
		     "")
		 ".html"))

(define (section-filename #!optional (n (current-node)))
  (let ((chunks (list-head (section-path n) %chunking-depth%)))
    (construct-filename (string-join "-" chunks))))

(define (section-anchor #!optional (n (current-node)))
  (let* ((path (section-path n))
	 (anchors (if (> %chunking-depth% (length path))
		      '()
		      (list-tail (section-path n) %chunking-depth%))))
    (if (null? anchors)
	#f
	(string-append "a" (string-join "-" anchors)))))

(define (section-href #!optional (n (current-node)) (always-include-filename? #f))
  (let* ((aname (section-anchor n))
	 (anchor (if aname (string-append "#" aname) ""))
	 (fname (section-filename n)))
    (if (or always-include-filename?
	    (not (equal? fname (section-filename))))
	(string-append fname anchor)
	anchor)))

(define (section-header-string #!optional (n (current-node)))
  (string-append (section-path-string n)
		 ". "
		 (walk-tree n '("heading"))))

(define (section-header #!optional (n (current-node)))
  (let* ((depth (+ (- (length (section-path n)) %chunking-depth%) 1))
	 (htag (string-append "H" (number->string (min (max depth 1) 6)))))
    (sexpr->sosofo
     `(,htag () ,(section-header-string n)))))

(define (section-separate? #!optional (n (current-node)))
  (<= (length (section-path n)) %chunking-depth%))

(define (notice-chunk-filename fn)
  (let ((notice (debug (string-append "Creating file: " fn))))
    fn))

(define (navigation-header-links-sexpr node)
  (let ((filter (lambda (x) (and (not (node-list-empty? x)) x)))
	(maybe-link (lambda (rel target)
			      (if target
				  (list `(link ((rel ,rel)
						(href ,(section-href target)))))
				  '()))))
    (let ((prev (filter (ipreced node)))
	  (next (filter (ifollow node)))
	  (up (filter (ancestor "section" node))))
      (append
       (list `(link ((rel top)
		     (href ,(construct-filename #f)))))
       (maybe-link 'prev prev)
       (maybe-link 'up up)
       (maybe-link 'next next)))))

(define %navigation-link-labels%
  '((top "Top")
    (prev "Previous")
    (next "Next")
    (up "Up")))

(define (navigation-body-links-sexpr node)
  (let* ((label-for (lambda (x) (cadr (assoc x %navigation-link-labels%))))
	 (filter (lambda (x) (and (not (node-list-empty? x)) x)))
	 (maybe-link (lambda (rel target)
		       (if target
			   (list '(0 nbsp) 
				 `(a ((href ,(section-href target)))
				     ,(label-for rel)))
			   '()))))
    (let ((prev (filter (ipreced node)))
	  (next (filter (ifollow node)))
	  (up (filter (ancestor "section" node))))
      (append
       (list `(a ((href ,(construct-filename #f)))
		 ,(label-for 'top)))
       (maybe-link 'prev prev)
       (maybe-link 'up up)
       (maybe-link 'next next)))))

(define (build-toc-sosofo #!optional (node (current-node)))
  (if (section-separate? node)
      (let ((nodelist (children node)))
	(sexprs->sosofo
	 (if (node-list-empty? (select-elements nodelist '(section)))
	     (empty-sosofo)
	     '(h2 () "Contents"))
	 `(ul ()
	      ,(with-mode toc-mode (process-node-list nodelist)))))
      (empty-sosofo)))

(define (html-header-sexpr #!rest titles)
  `(head ()
	 (link ((rel stylesheet)
		(type text/css)
		(href "galan-style.css")))
	 ,@(navigation-header-links-sexpr (current-node))
	 (title () ,@titles)))

(define (galandoc-title)
  (walk-root '(meta title (flattext))))

;---------------------------------------------------------------------------

(element galandoc
  (make entity
    system-id: (notice-chunk-filename (construct-filename #f))))

(element intro
  (sexpr->sosofo
   `(html ()
	  ,(html-header-sexpr (galandoc-title))
	  (body ()
		,@(navigation-body-links-sexpr (current-node))
		(hr ())
		(h1 () ,(galandoc-title))
		,(process-children)
		,(build-toc-sosofo (sgml-root-element))
		(hr ())
		,@(navigation-body-links-sexpr (current-node))
		))))

(mode toc-mode
  (element galandoc
    (process-children))

  (element section
    (sexpr->sosofo
     `(li ()
	  (a ((href ,(section-href (current-node) #t)))
	     ,(section-header-string))
	  ,(let ((subsections (select-elements (children (current-node)) '(section))))
	     (if (node-list-empty? subsections)
		 (empty-sosofo)
		 `(ul () ,(process-node-list subsections))))
	  )))

  (default (empty-sosofo)))

(element section
  (let ((content-sosofo (apply sexprs->sosofo
			       `(,@(map (lambda (aname)
					  (if aname
					      `(a ((name ,aname)))
					      (empty-sosofo)))
					(list (section-anchor)
					      (id (current-node))))
				 ,(section-header)
				 ,(build-toc-sosofo)
				 ,(process-children)))))
    (if (section-separate?)
	(make entity
	  system-id: (notice-chunk-filename (section-filename))
	  (sexpr->sosofo
	   `(html ()
		  ,(html-header-sexpr
		    (galandoc-title)
		    ": "
		    (walk-current '("heading")))
		  (body ()
			,@(navigation-body-links-sexpr (current-node))
			(hr ())
			,content-sosofo
			(hr ())
			,@(navigation-body-links-sexpr (current-node))
			))))
	content-sosofo)))

(element meta (empty-sosofo))
(element (meta title) (empty-sosofo))

(element p (make element gi: "p"))

(define (notebox #!rest sexprs)
  (sexpr->sosofo
   `(p ()
       (div ((class notebox))
	    ,@sexprs
	    ,(process-children)))))

(element warning (notebox '(font ((color red)) "Warning:" (0 nbsp))))
(element note (notebox '(font ((color blue)) "Note:" (0 nbsp))))

(element pre
  (sexpr->sosofo
   `(p ()
       (pre ()
	    ,(process-children)))))

(element b (make element gi: "b"))
(element i (make element gi: "i"))

(element emph
  (let* ((ancs (ancestors (current-node)))
	 (emphs (node-list-filter (lambda (x) (equal? (gi x) (normalize "emph"))) ancs)))
    (case (node-list-length emphs)
      ((0) (make element gi: "b"))
      ((1) (make element gi: "i"))
      ((2) (make element gi: "u"))
      (else (process-children)))))

(element code (make element gi: "tt" attributes: '(("CLASS" "CODE"))))

(element br (make empty-element gi: "br"))

(element ul (make element gi: "ul"))
(element ol (make element gi: "ol"))
(element li (make element gi: "li"))

(element link
  (sexpr->sosofo
   `(a ((href ,(walk-current '("url"))))
       ,(process-children))))

(element xref
  (let ((target (element-with-id (walk-current '("section"))
				 (sgml-root-element))))
    (if (node-list-empty? target)
	(node-list-error "Section name not present in xref" (current-node))
	(sexpr->sosofo
	 `(a ((href ,(section-href target)))
	     "Section "
	     ,(section-path-string target))))))

(element fig
  (sexpr->sosofo
   `(div ((align center))
	 (img ((src ,(walk-current '("src")))
	       (alt ,(walk-current '("alt"))))))))

(default
  (sexpr->sosofo
   `(font ((color red)
	   (x-dsl-unhandled-tag ,(gi)))
	  ,(process-children))))

</style-specification-body>
</style-specification>
</style-sheet>
