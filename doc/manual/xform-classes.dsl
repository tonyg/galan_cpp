;---------------------------------------------------------------------------
; Basic flow objects for SGML-to-SGML transformations.

(declare-flow-object-class element
  "UNREGISTERED::James Clark//Flow Object Class::element")

(declare-flow-object-class empty-element
  "UNREGISTERED::James Clark//Flow Object Class::empty-element")

(declare-flow-object-class document-type
  "UNREGISTERED::James Clark//Flow Object Class::document-type")

(declare-flow-object-class processing-instruction
  "UNREGISTERED::James Clark//Flow Object Class::processing-instruction")

(declare-flow-object-class entity
  "UNREGISTERED::James Clark//Flow Object Class::entity")

(declare-flow-object-class entity-ref
  "UNREGISTERED::James Clark//Flow Object Class::entity-ref")

(declare-flow-object-class formatting-instruction
  "UNREGISTERED::James Clark//Flow Object Class::formatting-instruction")

(declare-characteristic preserve-sdata?
  "UNREGISTERED::James Clark//Characteristic::preserve-sdata?" #t)

(define debug
  (external-procedure "UNREGISTERED::James Clark//Procedure::debug"))

(define read-entity
  (external-procedure "UNREGISTERED::James Clark//Procedure::read-entity"))

(define all-element-number
  (external-procedure "UNREGISTERED::James Clark//Procedure::all-element-number"))

;---------------------------------------------------------------------------
; My own stuff.

(define (stringify x)
  (cond
   ((string? x) x)
   ((number? x) (number->string x))
   ((symbol? x) (symbol->string x))
   (else
    (let* ((err1 (debug "Unknown thing in stringify (follows):"))
	   (err2 (my-debug x)))
      x))))

(define (sexpr->sosofo x)
  (cond
   ((pair? x)
    (let ((tag (car x))
	  (atts (cadr x))
	  (body (cddr x)))
      (if (number? tag) ; hack for entity-refs
	  (make entity-ref name: (stringify atts))
	  (let ((body-maybe-sosofo
		 (foldl1 (lambda (x y) (make sequence x y)) (map sexpr->sosofo body)))
		(attribute-list
		 (map (lambda (att) (list (stringify (car att))
					  (foldl (lambda (x y)
						   (string-append x (stringify y)))
						 ""
						 (cdr att))))
		      atts)))
	    (if (null? body-maybe-sosofo)
		(make empty-element
		  gi: (stringify tag)
		  attributes: attribute-list)
		(make element
		  gi: (stringify tag)
		  attributes: attribute-list
		  body-maybe-sosofo))))))
   ((sosofo? x) x)
   (else (literal (stringify x)))))

(define (sexprs->sosofo #!rest sexprs)
  (let ((result (foldl1 (lambda (x y) (make sequence x y)) (map sexpr->sosofo sexprs))))
    (if (null? result)
	(empty-sosofo)
	result)))

(define (walk-tree node path)
  (if (null? path)
      node
      (let ((instr (car path))
	    (rest (cdr path)))
	(cond
	 ; Extract an attribute
	 ((string? instr)
	  (or (attribute-string (normalize instr) node)
	      (debug (string-append "ATTRIBUTE NOT FOUND: " instr))))

	 ; Extract a child node
	 ((symbol? instr)
	  (let ((subnodes (select-elements (children node)
					   (normalize (symbol->string instr)))))
	    (if (node-list-empty? subnodes)
		(debug (string-append "SUBNODE NOT FOUND: " (symbol->string instr)))
		(walk-tree (node-list-first subnodes) rest))))

	 ; Extract either an indexed child node, or a "special" entry
	 ((and (pair? instr)
	       (symbol? (car instr)))
	  (cond
	   ; Extract a "special" entry
	   ((null? (cdr instr))
	    (case (car instr)
	      ; Extract all PCDATA from this node onward, ignoring all markup
	      ((flattext) (data node))

	      (else
	       (debug (string-append "Unknown special path element: " (stringify (car instr)))))))

	   ; Extract an indexed child node
	   ((and (number? (cadr instr))
		 (null? (cddr instr)))
	    (walk-tree (node-list-ref (select-elements (children node)
						       (normalize (symbol->string (car instr))))
				      (cadr instr))
		       rest))

	   (else
	    (debug "Unknown special format in walk-tree!"))))
	 (else
	  (debug "Unknown path type in walk-tree!"))))))

(define (walk-current path)
  (walk-tree (current-node) path))

(define (walk-root path)
  (walk-tree (sgml-root-element) path))

(define (string-join separator strings)
  (cond
   ((null? strings) "")
   ((null? (cdr strings)) (stringify (car strings)))
   (else
    (string-append (stringify (car strings))
		   separator
		   (string-join separator (cdr strings))))))

(define (list-split l k)
  (cond
   ((or (null? l)
	(zero? k))
    (list '() l))
   (else
    (let ((r (list-split (cdr l) (- k 1))))
      (cons (cons (car l) (car r))
	    (cdr r))))))

(define (list-head l k)
  (cond
   ((null? l) '())
   ((zero? k) '())
   (else
    (cons (car l) (list-head (cdr l) (- k 1))))))

(define (case-fold-lower s)
  (list->string (map char-downcase (string->list s))))

(define (case-fold-upper s)
  (list->string (map char-upcase (string->list s))))

;---------------------------------------------------------------------------
; Useful utilities, some from the DSSSL documentation at Mulberry Tech.

(define (compose f g)
  (lambda (x)
    (f (g x))))

; From a docbook transformer.
(define (normalize str)
  (if (string? str)
      (general-name-normalize str (current-node))
      str))

(define (sgml-root-element #!optional (grove-node (current-node)))
  ;; REFENTRY
  ;; PURP Returns the node that is the root element of the current document
  ;; DESC
  ;; Returns the node that is the root element of the current document
  ;; /DESC
  ;; /REFENTRY
  (node-property 'document-element (node-property 'grove-root grove-node)))

;; Map function `f' over node list `nl', returning an ordinary list.
;; (No node list constructor in Jade.)
(define (map-node-list->list f nl)
  (if (node-list-empty? nl)
      '()
      (cons (f (node-list-first nl))
            (map-node-list->list f (node-list-rest nl)))))

;; Fold left with function `f' over list `xs' with the given `zero'
;; value.  (Like DSSSL `reduce' but normal arg order.)
(define (foldl f zero xs)
  (if (null? xs)
      zero
      (foldl f (f zero (car xs)) (cdr xs))))

;; Fold left with list car as zero.
(define (foldl1 f xs)
  (cond ((null? xs)
         '())
        ((null? (cdr xs))
         (car xs))
        (else (foldl f (car xs) (cdr xs)))))

;; A version of debug that tries to print more helpful information
;; than `unknown object ...'.  Will need extending for any further
;; types added to Jade which don't have useful print methods.  Fixme:
;; should yield more information extracted from each type.
(define (my-debug x #!optional return-value)
  (debug (cond ((node-list? x)
                (if (node-list-empty? x)
                    (list 'empty-node-list x)
                    (list (if (named-node-list? x)
                              'named-node-list
                              'node-list)
                          (node-list-length x) x)))
               ((sosofo? x)
                (list 'sosofo x))
               ((procedure? x)
                (list 'procedure x))
               ((style? x)
                (list 'style x))
               ((address? x)
                (list 'address x))
               ((color? x)
                (list 'color x))
               ((color-space? x)
                (list 'color-space x))
               ((display-space? x)
                (list 'display-space x))
               ((inline-space? x)
                (list 'inline-space x))
               ((glyph-id? x)
                (list 'glyph-id x))
               ((glyph-subst-table? x)
                (list 'glyph-subst-table x))
               (else x))))
