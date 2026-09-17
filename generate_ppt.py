import sys
import os
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN
from pptx.enum.shapes import MSO_SHAPE

def build_presentation():
    prs = Presentation()
    # 16:9 Widescreen dimensions
    prs.slide_width = Inches(13.333)
    prs.slide_height = Inches(7.5)

    # Color Palette: Clean, professional, uncluttered
    NAVY = RGBColor(15, 23, 42)         # Primary Dark Header
    BLUE = RGBColor(37, 99, 235)        # Accent / Primary Blue
    DARK_GRAY = RGBColor(51, 65, 85)    # Body text
    LIGHT_GRAY = RGBColor(241, 245, 249)# Card fill
    BORDER_COLOR = RGBColor(203, 213, 225) # Card border
    WHITE = RGBColor(255, 255, 255)
    GREEN = RGBColor(16, 185, 129)      # Success / Objectives
    RED = RGBColor(220, 38, 38)         # Scope exclusions / errors
    CODE_BG = RGBColor(248, 250, 252)   # Code blocks

    blank_layout = prs.slide_layouts[6]

    def add_slide_header(slide, chapter_number, chapter_title):
        # White background
        bg = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0), Inches(0), Inches(13.333), Inches(7.5))
        bg.fill.solid()
        bg.fill.fore_color.rgb = WHITE
        bg.line.fill.background()

        # Top Blue accent strip
        strip = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0), Inches(0), Inches(13.333), Inches(0.12))
        strip.fill.solid()
        strip.fill.fore_color.rgb = BLUE
        strip.line.fill.background()

        # Chapter Number Tag
        tag_box = slide.shapes.add_textbox(Inches(0.8), Inches(0.4), Inches(11.7), Inches(0.35))
        tf_tag = tag_box.text_frame
        tf_tag.word_wrap = True
        tf_tag.margin_left = tf_tag.margin_top = tf_tag.margin_right = tf_tag.margin_bottom = 0
        p_tag = tf_tag.paragraphs[0]
        p_tag.text = chapter_number.upper()
        p_tag.font.size = Pt(11)
        p_tag.font.bold = True
        p_tag.font.color.rgb = BLUE

        # Chapter Title
        title_box = slide.shapes.add_textbox(Inches(0.8), Inches(0.75), Inches(11.7), Inches(0.65))
        tf_title = title_box.text_frame
        tf_title.word_wrap = True
        tf_title.margin_left = tf_title.margin_top = tf_title.margin_right = tf_title.margin_bottom = 0
        p_title = tf_title.paragraphs[0]
        p_title.text = chapter_title
        p_title.font.size = Pt(22)
        p_title.font.bold = True
        p_title.font.color.rgb = NAVY

    def create_card(slide, x, y, w, h, fill_color=LIGHT_GRAY, border_color=BORDER_COLOR):
        card = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(x), Inches(y), Inches(w), Inches(h))
        card.fill.solid()
        card.fill.fore_color.rgb = fill_color
        card.line.color.rgb = border_color
        card.line.width = Pt(1)
        return card

    # =========================================================================
    # SLIDE 1: TITLE SLIDE
    # =========================================================================
    s1 = prs.slides.add_slide(blank_layout)
    bg1 = s1.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0), Inches(0), Inches(13.333), Inches(7.5))
    bg1.fill.solid()
    bg1.fill.fore_color.rgb = NAVY
    bg1.line.fill.background()

    # Blue top accent bar
    top1 = s1.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0), Inches(0), Inches(13.333), Inches(0.18))
    top1.fill.solid()
    top1.fill.fore_color.rgb = BLUE
    top1.line.fill.background()

    tbox1 = s1.shapes.add_textbox(Inches(1.0), Inches(1.8), Inches(11.333), Inches(2.8))
    tf1 = tbox1.text_frame
    tf1.word_wrap = True

    p = tf1.paragraphs[0]
    p.text = "BCSE307L — COMPILER DESIGN LABORATORY PROJECT"
    p.font.size = Pt(13)
    p.font.bold = True
    p.font.color.rgb = RGBColor(147, 197, 253)

    p = tf1.add_paragraph()
    p.text = "TrafficRule"
    p.font.size = Pt(46)
    p.font.bold = True
    p.font.color.rgb = WHITE

    p = tf1.add_paragraph()
    p.text = "A Domain-Specific Language and Compiler for Traffic Rule Enforcement"
    p.font.size = Pt(20)
    p.font.color.rgb = RGBColor(203, 213, 225)

    # Info Card
    create_card(s1, 1.0, 4.8, 11.333, 1.5, fill_color=RGBColor(30, 41, 59), border_color=RGBColor(51, 65, 85))
    cbox1 = s1.shapes.add_textbox(Inches(1.3), Inches(5.05), Inches(10.7), Inches(1.0))
    ctf1 = cbox1.text_frame
    
    p = ctf1.paragraphs[0]
    p.text = "Student Name: Alan James     |     Register No: 24BCT0181"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = WHITE

    p = ctf1.add_paragraph()
    p.text = "Course: BCSE307L Compiler Design Lab     |     Review: Project Review 1 (Phase 1)"
    p.font.size = Pt(13)
    p.font.color.rgb = RGBColor(148, 163, 184)

    # =========================================================================
    # SLIDE 2: CHAPTER 1 — INTRODUCTION & MOTIVATION
    # =========================================================================
    s2 = prs.slides.add_slide(blank_layout)
    add_slide_header(s2, "Chapter 1", "Introduction & Motivation")

    # Left: Background & Problem Area
    create_card(s2, 0.8, 1.6, 5.6, 5.3)
    b2_1 = s2.shapes.add_textbox(Inches(1.05), Inches(1.85), Inches(5.1), Inches(4.8))
    tf = b2_1.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Background & Problem Area"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    pts = [
        "A compiler translates high-level user specifications into structured representations through defined stages (scanning, parsing, semantic checking).",
        "In academic coursework, compiler stages are often studied in isolation. TrafficRule integrates them into one coherent, working system.",
        "Traffic enforcement naturally follows an 'IF condition THEN action' structure, making it an intuitive domain for a lightweight Domain-Specific Language (DSL).",
        "Example traffic rule:\n   IF speed > 60 {\n       ISSUE_FINE\n   }"
    ]
    for pt in pts:
        p = tf.add_paragraph()
        p.text = "• " + pt
        p.font.size = Pt(12)
        p.font.color.rgb = DARK_GRAY

    # Right: Motivation
    create_card(s2, 6.8, 1.6, 5.7, 5.3)
    b2_2 = s2.shapes.add_textbox(Inches(7.05), Inches(1.85), Inches(5.2), Inches(4.8))
    tf = b2_2.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Why TrafficRule?"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    pts2 = [
        "Domain-Specific Clarity: Provides a simple, focused grammar for traffic operators without the excessive syntactic clutter of C or Python.",
        "Inspectable Pipeline: Every compiler stage (Lexer tokens, Parser AST, Error reporting) produces clear, demonstrable intermediate outputs.",
        "Focused Scope: Focuses strictly on core Compiler Design principles without distractions from hardware cameras or GPS hardware.",
        "Educational Demonstration: Allows live verification of LL(1) grammar compliance, deterministic tokenization, and syntax tree generation."
    ]
    for pt in pts2:
        p = tf.add_paragraph()
        p.text = "• " + pt
        p.font.size = Pt(12)
        p.font.color.rgb = DARK_GRAY

    # =========================================================================
    # SLIDE 3: CHAPTER 2 — PROBLEM STATEMENT & SCOPE
    # =========================================================================
    s3 = prs.slides.add_slide(blank_layout)
    add_slide_header(s3, "Chapter 2", "Problem Statement & Project Scope")

    # Left: Problem Statement
    create_card(s3, 0.8, 1.6, 5.6, 5.3)
    b3_1 = s3.shapes.add_textbox(Inches(1.05), Inches(1.85), Inches(5.1), Inches(4.8))
    tf = b3_1.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Problem Statement"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    pts = [
        "Traffic regulations rely on conditional checks (speed limits, signal compliance, crossing safety).",
        "General-purpose programming languages are overly complex for drafting and verifying simple safety regulations.",
        "The problem is to design a dedicated Domain-Specific Language (TrafficRule) and construct a complete compiler pipeline that can:",
        "  1. Ingest clean rule specifications from source files (*.tr).",
        "  2. Perform lexical scanning to produce typed tokens.",
        "  3. Validate syntax using an LL(1) deterministic grammar.",
        "  4. Construct an in-memory Abstract Syntax Tree (AST).",
        "  5. Provide informative line-and-column error diagnostics."
    ]
    for pt in pts:
        p = tf.add_paragraph()
        p.text = "• " if not pt.startswith("  ") else "   " + pt.strip()
        p.font.size = Pt(12)
        p.font.color.rgb = DARK_GRAY

    # Right: Scope Boundary (In-Scope vs Out-of-Scope)
    create_card(s3, 6.8, 1.6, 5.7, 5.3)
    b3_2 = s3.shapes.add_textbox(Inches(7.05), Inches(1.85), Inches(5.2), Inches(4.8))
    tf = b3_2.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Project Scope (lab5.pdf Section 6)"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    p = tf.add_paragraph()
    p.text = "IN-SCOPE (Compiler Construction Focus):"
    p.font.size = Pt(13)
    p.font.bold = True
    p.font.color.rgb = BLUE

    in_scope = [
        "Vehicle declarations, identifiers, and integer speeds.",
        "Traffic states: RED, GREEN, YELLOW, CROSSING, ROAD.",
        "Relational (>, <, ==, !=) and Logical (AND, OR) operators.",
        "Conditional IF / ELSE blocks and enforcement actions.",
        "Lexer, Recursive Descent Parser, and AST visualizer."
    ]
    for item in in_scope:
        p = tf.add_paragraph()
        p.text = "  ✔ " + item
        p.font.size = Pt(11)
        p.font.color.rgb = DARK_GRAY

    p = tf.add_paragraph()
    p.text = "\nOUT-OF-SCOPE (Non-Compiler Hardware/AI):"
    p.font.size = Pt(13)
    p.font.bold = True
    p.font.color.rgb = RED

    out_scope = [
        "Physical traffic cameras or license plate recognition (YOLO).",
        "GPS hardware tracking or live traffic signal controllers."
    ]
    for item in out_scope:
        p = tf.add_paragraph()
        p.text = "  ✖ " + item
        p.font.size = Pt(11)
        p.font.color.rgb = DARK_GRAY

    # =========================================================================
    # SLIDE 4: CHAPTER 3 — PROJECT OBJECTIVES
    # =========================================================================
    s4 = prs.slides.add_slide(blank_layout)
    add_slide_header(s4, "Chapter 3", "Project Objectives")

    col_w = 3.6
    gap = 0.4
    cards = [
        ("Phase 1: Design & Parsing (Review 1)", BLUE, [
            "Design a simple, readable grammar for traffic rules.",
            "Implement Lexical Analysis in C99 with position tracking.",
            "Implement Syntax Analysis using Top-Down Recursive Descent.",
            "Build an in-memory Abstract Syntax Tree (AST).",
            "Provide hierarchical ASCII tree visualization.",
            "Detect and report lexical & syntax errors with line/column."
        ]),
        ("Phase 2: Semantics & IR (Upcoming)", NAVY, [
            "Implement Semantic Analysis (type & state checks).",
            "Maintain a Symbol Table with scopes and attributes.",
            "Disallow undeclared variables and invalid operations.",
            "Generate Three-Address Code (TAC) and Quadruples.",
            "Implement compile-time Constant Folding & dead code elimination."
        ]),
        ("Phase 3: Execution & Testing", GREEN, [
            "Evaluate rules using simulated traffic telemetry.",
            "Report multi-stage compiler diagnostics clearly.",
            "Maintain zero external dependencies in pure C99.",
            "Provide comprehensive automated test suites (test_all.ps1)."
        ])
    ]
    for idx, (head, col, items) in enumerate(cards):
        x = 0.8 + idx * (col_w + gap)
        create_card(s4, x, 1.6, col_w, 5.3)
        box = s4.shapes.add_textbox(Inches(x + 0.2), Inches(1.85), Inches(col_w - 0.4), Inches(4.8))
        tf = box.text_frame
        tf.word_wrap = True
        p = tf.paragraphs[0]
        p.text = head
        p.font.size = Pt(15)
        p.font.bold = True
        p.font.color.rgb = col
        for it in items:
            p = tf.add_paragraph()
            p.text = "• " + it
            p.font.size = Pt(12)
            p.font.color.rgb = DARK_GRAY

    # =========================================================================
    # SLIDE 5: CHAPTER 4 — LITERATURE SURVEY & BACKGROUND STUDY
    # =========================================================================
    s5 = prs.slides.add_slide(blank_layout)
    add_slide_header(s5, "Chapter 4", "Literature Survey & Background Study")

    create_card(s5, 0.8, 1.6, 11.7, 5.3)
    b5 = s5.shapes.add_textbox(Inches(1.1), Inches(1.85), Inches(11.1), Inches(4.8))
    tf = b5.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Existing Approaches vs. The TrafficRule DSL Approach"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    studies = [
        ("General-Purpose Languages (Python / C / Java)",
         "Widely used for rule scripts, but introduce heavy syntactic baggage (headers, object models, memory management, imports). Not suited for clean domain-level policy expression or demonstrating the stages of a compiler."),
        ("Computer Vision & Sensor-Only Systems (YOLO / OpenCV)",
         "Focus exclusively on physical detection (vehicle tracking, plate OCR). However, they lack a formal programming language pipeline and cannot demonstrate formal grammar checking, AST generation, or compiler optimization passes."),
        ("TrafficRule Approach (Custom DSL & Compiler in C99)",
         "Bridges this gap by treating traffic policies as formal computer programs. Source text is scanned into tokens, verified against an LL(1) grammar, represented as an AST, and prepared for intermediate code generation with zero third-party dependencies.")
    ]
    for title, desc in studies:
        p_t = tf.add_paragraph()
        p_t.text = "• " + title
        p_t.font.size = Pt(13)
        p_t.font.bold = True
        p_t.font.color.rgb = BLUE
        p_d = tf.add_paragraph()
        p_d.text = "   " + desc
        p_d.font.size = Pt(12)
        p_d.font.color.rgb = DARK_GRAY

    # =========================================================================
    # SLIDE 6: CHAPTER 5 — SYSTEM DESIGN & ARCHITECTURE
    # =========================================================================
    s6 = prs.slides.add_slide(blank_layout)
    add_slide_header(s6, "Chapter 5", "System Design & Architecture")

    # Workflow cards reflecting Figure 1 & Module Table in lab5.pdf
    stages = [
        ("1. Source Code (*.tr)", "Traffic rules text (e.g., VEHICLE car, speed = 75, IF speed > 60 { ISSUE_FINE })", BLUE),
        ("2. Lexical Analyzer (lexer.c)", "Reads characters, strips comments (#), tracks line/col, outputs typed Token Stream", NAVY),
        ("3. Syntax Analyzer (parser.c)", "Validates token order using Top-Down Recursive Descent parser (LL(1) grammar)", NAVY),
        ("4. Abstract Syntax Tree (parser.h)", "Builds hierarchical tree in memory; provides ASCII visual tree formatting", NAVY),
        ("5. Diagnostic / Error Handler", "Catches lexical and syntax mistakes with line numbers, column offsets, and pointers", GREEN)
    ]
    y_start = 1.6
    for idx, (name, detail, col) in enumerate(stages):
        y = y_start + idx * 1.05
        create_card(s6, 0.8, y, 11.7, 0.9, fill_color=LIGHT_GRAY if idx % 2 == 0 else RGBColor(238, 242, 255))
        box = s6.shapes.add_textbox(Inches(1.1), Inches(y + 0.12), Inches(11.1), Inches(0.7))
        tf = box.text_frame
        p = tf.paragraphs[0]
        p.text = name
        p.font.size = Pt(14)
        p.font.bold = True
        p.font.color.rgb = col
        p2 = tf.add_paragraph()
        p2.text = detail
        p2.font.size = Pt(11)
        p2.font.color.rgb = DARK_GRAY

    # =========================================================================
    # SLIDE 7: CHAPTER 6 — METHODOLOGY & LANGUAGE SPECIFICATION
    # =========================================================================
    s7 = prs.slides.add_slide(blank_layout)
    add_slide_header(s7, "Chapter 6", "Methodology & Language Specification")

    # Left: Language Lexicon
    create_card(s7, 0.8, 1.6, 5.6, 5.3)
    b7_1 = s7.shapes.add_textbox(Inches(1.05), Inches(1.85), Inches(5.1), Inches(4.8))
    tf = b7_1.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Language Lexicon (lab5.pdf Sec 11)"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    lexicon = [
        ("Keywords", "VEHICLE, IF, ELSE, AND, OR"),
        ("Traffic States", "RED, GREEN, YELLOW, CROSSING, ROAD"),
        ("Rule Actions", "ISSUE_FINE, FLAG_VIOLATION, ALLOW"),
        ("Operators", "Assign:  =   |   Relational:  >,  <,  >=,  <=,  ==,  !="),
        ("Delimiters", "Block braces: {  }"),
        ("Identifiers", "Variable names: speed, signal, position, car"),
        ("Literals", "Integer values: 60, 75, 100")
    ]
    for cat, val in lexicon:
        p = tf.add_paragraph()
        p.text = f"• {cat}: "
        p.font.size = Pt(12)
        p.font.bold = True
        p.font.color.rgb = BLUE
        p.text += val
        p.font.color.rgb = DARK_GRAY

    # Right: Formal LL(1) Grammar
    create_card(s7, 6.8, 1.6, 5.7, 5.3)
    b7_2 = s7.shapes.add_textbox(Inches(7.05), Inches(1.85), Inches(5.2), Inches(4.8))
    tf = b7_2.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Formal LL(1) Grammar (EBNF)"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    grammar_rules = [
        "program      -> statement*",
        "statement    -> declaration | assignment | if_stmt | action",
        "declaration  -> 'VEHICLE' identifier",
        "assignment   -> identifier '=' value",
        "if_stmt      -> 'IF' condition block ('ELSE' block)?",
        "condition    -> and_expr ('OR' and_expr)*",
        "and_expr     -> rel_expr ('AND' rel_expr)*",
        "rel_expr     -> value relational_op value",
        "block        -> '{' statement* '}'",
        "action       -> 'ISSUE_FINE' | 'FLAG_VIOLATION' | 'ALLOW'",
        "value        -> identifier | number | traffic_state"
    ]
    for gr in grammar_rules:
        p = tf.add_paragraph()
        p.text = gr
        p.font.size = Pt(10.5)
        p.font.name = "Consolas"
        p.font.color.rgb = NAVY

    # =========================================================================
    # SLIDE 8: CHAPTER 7 — COMPILER DESIGN CONCEPTS USED
    # =========================================================================
    s8 = prs.slides.add_slide(blank_layout)
    add_slide_header(s8, "Chapter 7", "Compiler Design Concepts Used")

    concepts = [
        ("Deterministic Lexical Scanning (DFA)",
         "The lexer processes source characters into discrete tokens using finite-state logic. Automatically tracks line numbers and column offsets to support accurate error reporting.",
         BLUE),
        ("Predictive Recursive Descent Parsing",
         "The parser is implemented via mutually recursive C functions (parse_statement, parse_if, parse_condition). Grammar non-terminals map directly to C functions without external tools.",
         NAVY),
        ("Deterministic LL(1) Grammar",
         "The grammar is deterministic with 1 lookahead token. Statement production alternatives have disjoint FIRST sets (VEHICLE, identifier, IF, action), eliminating backtracking.",
         GREEN),
        ("Precedence-By-Grammar Architecture",
         "Operator precedence is encoded directly in the grammar hierarchy (OR -> AND -> Relational), guaranteeing correct associativity without requiring parentheses.",
         DARK_GRAY)
    ]
    for idx, (title, desc, color) in enumerate(concepts):
        y = 1.6 + idx * 1.3
        create_card(s8, 0.8, y, 11.7, 1.15)
        box = s8.shapes.add_textbox(Inches(1.05), Inches(y + 0.15), Inches(11.1), Inches(0.9))
        tf = box.text_frame
        tf.word_wrap = True
        p = tf.paragraphs[0]
        p.text = title
        p.font.size = Pt(15)
        p.font.bold = True
        p.font.color.rgb = color
        p2 = tf.add_paragraph()
        p2.text = desc
        p2.font.size = Pt(12)
        p2.font.color.rgb = DARK_GRAY

    # =========================================================================
    # SLIDE 9: CHAPTER 8 — IMPLEMENTATION DETAILS
    # =========================================================================
    s9 = prs.slides.add_slide(blank_layout)
    add_slide_header(s9, "Chapter 8", "Implementation Details (Pure C99)")

    # Left: Modular File Organization
    create_card(s9, 0.8, 1.6, 5.6, 5.3)
    b9_1 = s9.shapes.add_textbox(Inches(1.05), Inches(1.85), Inches(5.1), Inches(4.8))
    tf = b9_1.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Modular Source Organization"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    c_files = [
        ("src/lexer.h & src/lexer.c",
         "Defines TokenType enums and Token structs. Implements lexer_init, lexer_next_token, keyword hash-lookup, and line/column tracking."),
        ("src/parser.h & src/parser.c",
         "Defines ASTNode structs (Program, Decl, Assign, If, Condition, Action). Implements recursive descent parser and hierarchical tree printer."),
        ("src/main.c",
         "Entry point driver. Handles command-line flags (--tokens, --ast, --all), loads source files, and drives compiler execution.")
    ]
    for fn, desc in c_files:
        p = tf.add_paragraph()
        p.text = fn
        p.font.size = Pt(13)
        p.font.bold = True
        p.font.color.rgb = BLUE
        p2 = tf.add_paragraph()
        p2.text = "  " + desc
        p2.font.size = Pt(11)
        p2.font.color.rgb = DARK_GRAY

    # Right: Technical Highlights
    create_card(s9, 6.8, 1.6, 5.7, 5.3)
    b9_2 = s9.shapes.add_textbox(Inches(7.05), Inches(1.85), Inches(5.2), Inches(4.8))
    tf = b9_2.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Engineering Highlights"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    eng_pts = [
        "Language & Standard: Pure C (C99 standard).",
        "Compiler Toolchain: MinGW GCC 6.3.0.",
        "Zero External Dependencies: Written completely from scratch — no Flex, Lex, Bison, or Yacc required.",
        "Zero Warning Build: Compiles cleanly with flags -Wall -Wextra without any warnings or errors.",
        "Memory Safety: Recursive ast_free() routine deallocates all AST nodes, preventing memory leaks.",
        "Build Automation: One-click build.bat compiles directly to trafficrule.exe."
    ]
    for pt in eng_pts:
        p = tf.add_paragraph()
        p.text = "• " + pt
        p.font.size = Pt(12)
        p.font.color.rgb = DARK_GRAY

    # =========================================================================
    # SLIDE 10: CHAPTER 9 — TESTING & RESULTS
    # =========================================================================
    s10 = prs.slides.add_slide(blank_layout)
    add_slide_header(s10, "Chapter 9", "Testing & Prototype Results (Review 1)")

    # Left: Test Suite Summary
    create_card(s10, 0.8, 1.6, 5.6, 5.3)
    b10_1 = s10.shapes.add_textbox(Inches(1.05), Inches(1.85), Inches(5.1), Inches(4.8))
    tf = b10_1.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Test Matrix Verification (4/4 Passed)"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    tests = [
        ("Test 1: sample1_speed.tr", "Speed limit condition verified. Token stream and AST generated accurately.", GREEN),
        ("Test 2: sample2_compound.tr", "Compound AND expression verified. Nested binary AST generated.", GREEN),
        ("Test 3: Lexical Error Catching", "Input '@' symbol properly caught with line 3, col 12 error diagnostics.", GREEN),
        ("Test 4: Syntax Error Catching", "Malformed 'IF speed > {' caught with line 5 missing operand diagnostics.", GREEN)
    ]
    for name, desc, col in tests:
        p = tf.add_paragraph()
        p.text = "✔ " + name
        p.font.size = Pt(13)
        p.font.bold = True
        p.font.color.rgb = col
        p2 = tf.add_paragraph()
        p2.text = "    " + desc
        p2.font.size = Pt(11)
        p2.font.color.rgb = DARK_GRAY

    # Right: Prototype Output Snippet
    create_card(s10, 6.8, 1.6, 5.7, 5.3)
    b10_2 = s10.shapes.add_textbox(Inches(7.05), Inches(1.85), Inches(5.2), Inches(4.8))
    tf = b10_2.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Generated AST Output (Sample 1)"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    ast_text = (
        "Program (3 statements)\n"
        "  Declaration: VEHICLE car\n"
        "  Assignment: speed =\n"
        "    LiteralInt: 75\n"
        "  IF Statement\n"
        "    [Condition]:\n"
        "      BinaryOp: >\n"
        "        Identifier: speed\n"
        "        LiteralInt: 60\n"
        "    [Then Block]:\n"
        "      Program (1 statements)\n"
        "        Action: ISSUE_FINE"
    )
    p = tf.add_paragraph()
    p.text = ast_text
    p.font.size = Pt(11)
    p.font.name = "Consolas"
    p.font.color.rgb = NAVY

    # =========================================================================
    # SLIDE 11: CHAPTER 10 — CONCLUSION
    # =========================================================================
    s11 = prs.slides.add_slide(blank_layout)
    add_slide_header(s11, "Chapter 10", "Conclusion")

    create_card(s11, 0.8, 1.6, 11.7, 5.3)
    b11 = s11.shapes.add_textbox(Inches(1.1), Inches(1.85), Inches(11.1), Inches(4.8))
    tf = b11.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Review 1 Accomplishments & Summary"
    p.font.size = Pt(17)
    p.font.bold = True
    p.font.color.rgb = NAVY

    concl_pts = [
        "Designed and formalized the TrafficRule language specification, grammar, and lexicon specifically for traffic safety rules.",
        "Built a working Lexical Analyzer from scratch in C99 that converts character streams into tokens, skips comments, and tracks line/column positions.",
        "Built a predictive Recursive Descent Parser adhering to LL(1) formal grammar specifications, constructing hierarchical in-memory ASTs.",
        "Implemented robust, stage-specific error diagnostics providing clear messages with precise line and column markers.",
        "Successfully verified the Phase 1 prototype against multiple test cases, meeting all evaluation criteria for Project Review 1."
    ]
    for cp in concl_pts:
        p = tf.add_paragraph()
        p.text = "• " + cp
        p.font.size = Pt(13.5)
        p.font.color.rgb = DARK_GRAY

    # =========================================================================
    # SLIDE 12: CHAPTER 11 — FUTURE ENHANCEMENTS
    # =========================================================================
    s12 = prs.slides.add_slide(blank_layout)
    add_slide_header(s12, "Chapter 11", "Future Enhancements (Project Roadmap)")

    f_cards = [
        ("Phase 2: Semantic Analysis & Symbol Table", BLUE, [
            "Type checking (prevent speed == RED).",
            "Symbol table to track types, scopes, and variable values.",
            "Undeclared identifier detection.",
            "Type incompatibility error reporting."
        ]),
        ("Phase 2: Intermediate Code & Optimization", NAVY, [
            "Three-Address Code (TAC) generation with temporaries (t1, t2).",
            "Quadruples representation table.",
            "Compile-time Constant Folding.",
            "Dead branch and dead code elimination."
        ]),
        ("Phase 3: Execution & Final Submission", GREEN, [
            "Lightweight interpreter virtual machine.",
            "Simulated vehicle telemetry evaluation.",
            "JSON/CSV batch violation test inputs.",
            "Complete final project documentation."
        ])
    ]
    for idx, (title, col, items) in enumerate(f_cards):
        x = 0.8 + idx * (col_w + gap)
        create_card(s12, x, 1.6, col_w, 5.3)
        box = s12.shapes.add_textbox(Inches(x + 0.2), Inches(1.85), Inches(col_w - 0.4), Inches(4.8))
        tf = box.text_frame
        tf.word_wrap = True
        p = tf.paragraphs[0]
        p.text = title
        p.font.size = Pt(14)
        p.font.bold = True
        p.font.color.rgb = col
        for it in items:
            p = tf.add_paragraph()
            p.text = "• " + it
            p.font.size = Pt(12)
            p.font.color.rgb = DARK_GRAY

    # =========================================================================
    # SLIDE 13: REFERENCES
    # =========================================================================
    s13 = prs.slides.add_slide(blank_layout)
    add_slide_header(s13, "Section 20", "References & Bibliography")

    create_card(s13, 0.8, 1.6, 11.7, 5.3)
    b13 = s13.shapes.add_textbox(Inches(1.1), Inches(1.85), Inches(11.1), Inches(4.8))
    tf = b13.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Academic References & Documentation"
    p.font.size = Pt(17)
    p.font.bold = True
    p.font.color.rgb = NAVY

    references = [
        "1. Alfred V. Aho, Monica S. Lam, Ravi Sethi, and Jeffrey D. Ullman — 'Compilers: Principles, Techniques, and Tools' (2nd Edition, Pearson / Addison-Wesley).",
        "2. Kenneth C. Louden — 'Compiler Construction: Principles and Practice' (Course Technology / Cengage Learning).",
        "3. Andrew W. Appel — 'Modern Compiler Implementation in C' (Cambridge University Press).",
        "4. BCSE307L Compiler Design Laboratory Course Manual & Project Guidelines, Fall Semester 2026–27, VIT.",
        "5. TrafficRule Project Repository: Complete C99 source code, test suites, and project documentation."
    ]
    for r in references:
        p = tf.add_paragraph()
        p.text = r
        p.font.size = Pt(13)
        p.font.color.rgb = DARK_GRAY

    # =========================================================================
    # SLIDE 14: APPENDIX — CODE DETAILS & SUPPLEMENTARY MATERIAL
    # =========================================================================
    s14 = prs.slides.add_slide(blank_layout)
    add_slide_header(s14, "Section 21", "Appendix — Execution Details & Commands")

    # Left: CLI Flags & Commands
    create_card(s14, 0.8, 1.6, 5.6, 5.3)
    b14_1 = s14.shapes.add_textbox(Inches(1.05), Inches(1.85), Inches(5.1), Inches(4.8))
    tf = b14_1.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Execution Commands & Flags"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = NAVY

    cmds = [
        ("Build Compiler", ".\\build.bat\n(Compiles src/ into trafficrule.exe)"),
        ("Print Token Stream", ".\\trafficrule.exe --tokens examples\\sample1_speed.tr"),
        ("Print Abstract Syntax Tree", ".\\trafficrule.exe --ast examples\\sample1_speed.tr"),
        ("Run Full Pipeline", ".\\trafficrule.exe --all examples\\sample1_speed.tr"),
        ("Run Automated Test Suite", ".\\test_all.ps1\n(Executes all valid and invalid test cases)")
    ]
    for label, cmd_str in cmds:
        p = tf.add_paragraph()
        p.text = f"• {label}:"
        p.font.size = Pt(12)
        p.font.bold = True
        p.font.color.rgb = BLUE
        p2 = tf.add_paragraph()
        p2.text = cmd_str
        p2.font.size = Pt(10.5)
        p2.font.name = "Consolas"
        p2.font.color.rgb = DARK_GRAY

    # Right: Thank you / Q&A Card
    create_card(s14, 6.8, 1.6, 5.7, 5.3, fill_color=RGBColor(241, 245, 249))
    b14_2 = s14.shapes.add_textbox(Inches(7.1), Inches(2.3), Inches(5.1), Inches(4.0))
    tf = b14_2.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = "Thank You!"
    p.font.size = Pt(32)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.alignment = PP_ALIGN.CENTER

    p = tf.add_paragraph()
    p.text = "\nQuestions & Discussion\n"
    p.font.size = Pt(20)
    p.font.bold = True
    p.font.color.rgb = BLUE
    p.alignment = PP_ALIGN.CENTER

    p = tf.add_paragraph()
    p.text = "Alan James | 24BCT0181\nBCSE307L Compiler Design Lab"
    p.font.size = Pt(14)
    p.font.color.rgb = DARK_GRAY
    p.alignment = PP_ALIGN.CENTER

    # Save presentation
    output_file = "TrafficRule_Presentation.pptx"
    prs.save(output_file)
    print(f"Presentation successfully created: {output_file} ({len(prs.slides)} slides)")

if __name__ == "__main__":
    build_presentation()
