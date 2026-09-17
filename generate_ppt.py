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

    # Clean, High-Contrast Color Palette
    NAVY = RGBColor(15, 23, 42)         # Primary Dark Blue/Slate (Headers)
    BLUE = RGBColor(37, 99, 235)        # Accent Blue (Tags, Highlights)
    DARK_TEXT = RGBColor(30, 41, 59)    # High-contrast readable body text
    LIGHT_BG = RGBColor(248, 250, 252)  # Slide card background
    BORDER_COLOR = RGBColor(203, 213, 225) # Card border
    WHITE = RGBColor(255, 255, 255)
    GREEN = RGBColor(16, 185, 129)      # Pass / Objectives
    RED = RGBColor(220, 38, 38)         # Out-of-scope / Errors
    BLUE_BG = RGBColor(239, 246, 255)   # Light blue tint

    blank_layout = prs.slide_layouts[6]

    def add_header(slide, chapter_tag, title_text):
        # Clean white background
        bg = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0), Inches(0), Inches(13.333), Inches(7.5))
        bg.fill.solid()
        bg.fill.fore_color.rgb = WHITE
        bg.line.fill.background()

        # Top Blue accent strip
        strip = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0), Inches(0), Inches(13.333), Inches(0.12))
        strip.fill.solid()
        strip.fill.fore_color.rgb = BLUE
        strip.line.fill.background()

        # Chapter Number Tag (Big & bold)
        tag_box = slide.shapes.add_textbox(Inches(0.8), Inches(0.35), Inches(11.7), Inches(0.35))
        tf_tag = tag_box.text_frame
        tf_tag.word_wrap = True
        tf_tag.margin_left = tf_tag.margin_top = tf_tag.margin_right = tf_tag.margin_bottom = 0
        p_tag = tf_tag.paragraphs[0]
        p_tag.text = chapter_tag.upper()
        p_tag.font.size = Pt(13)
        p_tag.font.bold = True
        p_tag.font.color.rgb = BLUE

        # Main Slide Title (Large, clear, easily readable)
        title_box = slide.shapes.add_textbox(Inches(0.8), Inches(0.72), Inches(11.7), Inches(0.65))
        tf_title = title_box.text_frame
        tf_title.word_wrap = True
        tf_title.margin_left = tf_title.margin_top = tf_title.margin_right = tf_title.margin_bottom = 0
        p_title = tf_title.paragraphs[0]
        p_title.text = title_text
        p_title.font.size = Pt(26)
        p_title.font.bold = True
        p_title.font.color.rgb = NAVY

    def add_card(slide, x, y, w, h, fill_color=LIGHT_BG, border_color=BORDER_COLOR):
        card = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(x), Inches(y), Inches(w), Inches(h))
        card.fill.solid()
        card.fill.fore_color.rgb = fill_color
        card.line.color.rgb = border_color
        card.line.width = Pt(1.2)
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
    top1 = s1.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0), Inches(0), Inches(13.333), Inches(0.2))
    top1.fill.solid()
    top1.fill.fore_color.rgb = BLUE
    top1.line.fill.background()

    tbox1 = s1.shapes.add_textbox(Inches(1.0), Inches(1.5), Inches(11.333), Inches(3.2))
    tf1 = tbox1.text_frame
    tf1.word_wrap = True

    p = tf1.paragraphs[0]
    p.text = "BCSE307L - COMPILER DESIGN LABORATORY PROJECT"
    p.font.size = Pt(15)
    p.font.bold = True
    p.font.color.rgb = RGBColor(147, 197, 253)
    p.space_after = Pt(12)

    p = tf1.add_paragraph()
    p.text = "TrafficRule"
    p.font.size = Pt(54)
    p.font.bold = True
    p.font.color.rgb = WHITE
    p.space_after = Pt(14)

    p = tf1.add_paragraph()
    p.text = "A Domain-Specific Language and Compiler for Traffic Rule Enforcement"
    p.font.size = Pt(22)
    p.font.color.rgb = RGBColor(203, 213, 225)

    # Student details card
    add_card(s1, 1.0, 5.0, 11.333, 1.6, fill_color=RGBColor(30, 41, 59), border_color=RGBColor(51, 65, 85))
    cbox1 = s1.shapes.add_textbox(Inches(1.4), Inches(5.25), Inches(10.5), Inches(1.1))
    ctf1 = cbox1.text_frame
    
    p = ctf1.paragraphs[0]
    p.text = "Student Name: Alan James     |     Register No: 24BCT0181"
    p.font.size = Pt(18)
    p.font.bold = True
    p.font.color.rgb = WHITE
    p.space_after = Pt(8)

    p = ctf1.add_paragraph()
    p.text = "Course: BCSE307L Compiler Design Lab     |     Review: Project Review 1 (Phase 1)"
    p.font.size = Pt(15)
    p.font.color.rgb = RGBColor(148, 163, 184)

    # =========================================================================
    # SLIDE 2: CHAPTER 1 - INTRODUCTION & MOTIVATION
    # =========================================================================
    s2 = prs.slides.add_slide(blank_layout)
    add_header(s2, "Chapter 1", "Introduction & Motivation")

    # Left: Background & Problem Area
    add_card(s2, 0.8, 1.55, 5.6, 5.4)
    b2_1 = s2.shapes.add_textbox(Inches(1.1), Inches(1.8), Inches(5.0), Inches(4.9))
    tf = b2_1.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Background & Concept"
    p.font.size = Pt(20)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(14)

    pts = [
        "A Domain-Specific Language (DSL) created specifically for traffic safety rules.",
        "Allows traffic operators to specify road conditions and safety policies naturally.",
        "Example rule:\n   IF speed > 60 {\n       ISSUE_FINE\n   }",
        "Translates high-level safety rules into executable logic through formal compiler stages."
    ]
    for pt in pts:
        p = tf.add_paragraph()
        p.text = "• " + pt
        p.font.size = Pt(16)
        p.font.color.rgb = DARK_TEXT
        p.space_after = Pt(10)

    # Right: Motivation
    add_card(s2, 6.8, 1.55, 5.7, 5.4)
    b2_2 = s2.shapes.add_textbox(Inches(7.1), Inches(1.8), Inches(5.1), Inches(4.9))
    tf = b2_2.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Why TrafficRule?"
    p.font.size = Pt(20)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(14)

    pts2 = [
        "Putting Compiler Stages Together: In lab coursework, stages like scanning and parsing are studied separately. This project unifies them.",
        "Natural Condition-Action Fit: Traffic rules inherently map to 'IF condition THEN action'.",
        "Inspectable Pipeline: Every stage (Tokens, AST, Errors) produces clear intermediate outputs for demonstration.",
        "Pure Compiler Focus: Zero dependencies on cameras or GPS hardware."
    ]
    for pt in pts2:
        p = tf.add_paragraph()
        p.text = "• " + pt
        p.font.size = Pt(16)
        p.font.color.rgb = DARK_TEXT
        p.space_after = Pt(10)

    # =========================================================================
    # SLIDE 3: CHAPTER 2 - PROBLEM STATEMENT & SCOPE
    # =========================================================================
    s3 = prs.slides.add_slide(blank_layout)
    add_header(s3, "Chapter 2", "Problem Statement & Project Scope")

    # Left: Problem Statement
    add_card(s3, 0.8, 1.55, 5.6, 5.4)
    b3_1 = s3.shapes.add_textbox(Inches(1.1), Inches(1.8), Inches(5.0), Inches(4.9))
    tf = b3_1.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Problem Statement"
    p.font.size = Pt(20)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(14)

    ps_pts = [
        "Traffic safety policies are written as conditions (speed limits, signal compliance).",
        "General programming languages (C/Python) have excessive syntactic complexity for rule writers.",
        "The objective is to design a dedicated DSL and build an end-to-end compiler to:",
        "  1. Read traffic rule source code (*.tr)",
        "  2. Lexically scan characters into typed tokens",
        "  3. Validate syntax using an LL(1) grammar",
        "  4. Construct an Abstract Syntax Tree (AST)",
        "  5. Report exact line & column errors"
    ]
    for pt in ps_pts:
        p = tf.add_paragraph()
        p.text = "• " if not pt.startswith("  ") else "   " + pt.strip()
        p.font.size = Pt(15.5)
        p.font.color.rgb = DARK_TEXT
        p.space_after = Pt(8)

    # Right: Scope
    add_card(s3, 6.8, 1.55, 5.7, 5.4)
    b3_2 = s3.shapes.add_textbox(Inches(7.1), Inches(1.8), Inches(5.1), Inches(4.9))
    tf = b3_2.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Project Scope (lab5.pdf Section 6)"
    p.font.size = Pt(20)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(12)

    p = tf.add_paragraph()
    p.text = "IN-SCOPE (Compiler Construction Focus):"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = BLUE
    p.space_after = Pt(6)

    in_scope = [
        "Vehicle declarations, identifiers, integer speeds",
        "Traffic states: RED, GREEN, YELLOW, CROSSING",
        "Relational (>, <, ==) and Logical (AND, OR) operators",
        "Conditional IF / ELSE blocks & Action enforcement",
        "Custom Lexer, Recursive Descent Parser & AST"
    ]
    for it in in_scope:
        p = tf.add_paragraph()
        p.text = "  ✔ " + it
        p.font.size = Pt(14)
        p.font.color.rgb = DARK_TEXT
        p.space_after = Pt(4)

    p = tf.add_paragraph()
    p.text = "\nOUT-OF-SCOPE (Hardware / External AI):"
    p.font.size = Pt(16)
    p.font.bold = True
    p.font.color.rgb = RED
    p.space_after = Pt(6)

    out_scope = [
        "Physical traffic cameras or plate recognition (YOLO)",
        "GPS hardware tracking or signal controller hardware"
    ]
    for it in out_scope:
        p = tf.add_paragraph()
        p.text = "  ✖ " + it
        p.font.size = Pt(14)
        p.font.color.rgb = DARK_TEXT
        p.space_after = Pt(4)

    # =========================================================================
    # SLIDE 4: CHAPTER 3 - PROJECT OBJECTIVES
    # =========================================================================
    s4 = prs.slides.add_slide(blank_layout)
    add_header(s4, "Chapter 3", "Project Objectives")

    col_w = 3.6
    gap = 0.4
    cards = [
        ("Phase 1: Frontend (Current)", BLUE, [
            "Design clean grammar for traffic rules.",
            "Build pure C Lexer with line/col tracking.",
            "Build Top-Down Recursive Descent Parser.",
            "Construct in-memory AST and ASCII visualizer.",
            "Implement precise line/column error reporting."
        ]),
        ("Phase 2: Middle-End", NAVY, [
            "Implement Semantic Analysis (type checking).",
            "Build Symbol Table with scopes and values.",
            "Disallow undeclared variables/invalid states.",
            "Generate Three-Address Code (TAC) & Quadruples.",
            "Implement Constant Folding & Dead Code removal."
        ]),
        ("Phase 3: Backend", GREEN, [
            "Evaluate rules using simulated traffic telemetry.",
            "Support batch violation processing.",
            "Maintain 0 external dependencies (pure C99).",
            "Provide automated test suite verification."
        ])
    ]
    for idx, (head, col, items) in enumerate(cards):
        x = 0.8 + idx * (col_w + gap)
        add_card(s4, x, 1.55, col_w, 5.4)
        box = s4.shapes.add_textbox(Inches(x + 0.2), Inches(1.8), Inches(col_w - 0.4), Inches(4.9))
        tf = box.text_frame
        tf.word_wrap = True
        
        p = tf.paragraphs[0]
        p.text = head
        p.font.size = Pt(18)
        p.font.bold = True
        p.font.color.rgb = col
        p.space_after = Pt(14)
        
        for it in items:
            p = tf.add_paragraph()
            p.text = "• " + it
            p.font.size = Pt(15.5)
            p.font.color.rgb = DARK_TEXT
            p.space_after = Pt(10)

    # =========================================================================
    # SLIDE 5: CHAPTER 4 - LITERATURE SURVEY & BACKGROUND STUDY
    # =========================================================================
    s5 = prs.slides.add_slide(blank_layout)
    add_header(s5, "Chapter 4", "Literature Survey & Background Study")

    add_card(s5, 0.8, 1.55, 11.7, 5.4)
    b5 = s5.shapes.add_textbox(Inches(1.1), Inches(1.8), Inches(11.1), Inches(4.9))
    tf = b5.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Comparison: Existing Approaches vs. TrafficRule Approach"
    p.font.size = Pt(20)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(16)

    studies = [
        ("General-Purpose Languages (Python / C / Java)",
         "Widely used for scripting rules, but feature heavy syntactic baggage (classes, loops, type annotations, imports). Impractical for non-programmer rule authors and cannot isolate compiler phases."),
        ("Computer Vision & Sensor-Only Systems (YOLO / OpenCV)",
         "Focus strictly on physical vehicle detection and license plate recognition. They do not provide a formal programming language pipeline, grammatical syntax validation, AST, or intermediate optimizations."),
        ("TrafficRule Approach (Custom DSL & Pure C99 Compiler)",
         "Combines the simplicity of a dedicated traffic grammar with the academic rigor of compiler construction. Source text is tokenized, parsed via LL(1) recursive descent, and converted to an inspectable AST with zero third-party dependencies.")
    ]
    for title, desc in studies:
        p_t = tf.add_paragraph()
        p_t.text = "• " + title
        p_t.font.size = Pt(17)
        p_t.font.bold = True
        p_t.font.color.rgb = BLUE
        p_t.space_after = Pt(4)
        
        p_d = tf.add_paragraph()
        p_d.text = "   " + desc
        p_d.font.size = Pt(15)
        p_d.font.color.rgb = DARK_TEXT
        p_d.space_after = Pt(12)

    # =========================================================================
    # SLIDE 6: CHAPTER 5 - COMPLETE ARCHITECTURE (Figure 1 in lab5.pdf)
    # =========================================================================
    s6 = prs.slides.add_slide(blank_layout)
    add_header(s6, "Chapter 5", "Complete Compiler Architecture (lab5.pdf Figure 1)")

    # Flowchart visualization: 8 connected horizontal cards
    pipeline = [
        ("Source Program", "*.tr file\nspeed = 75", BLUE),
        ("Lexer", "lexer.c\nToken Stream", NAVY),
        ("Parser", "parser.c\nLL(1) Grammar", NAVY),
        ("AST", "parser.h\nSyntax Tree", NAVY),
        ("Semantic Analyzer", "Type check &\nSymbol Table", NAVY),
        ("IR Generator", "Three-Address\nCode (TAC)", NAVY),
        ("Optimizer", "Constant fold\nDead code", NAVY),
        ("Execution Engine", "Simulated\nEnforcement", GREEN)
    ]
    box_w = 1.3
    gap_x = 0.16
    start_x = 0.8
    y_pos = 1.6

    for idx, (p_title, p_sub, p_col) in enumerate(pipeline):
        cur_x = start_x + idx * (box_w + gap_x)
        add_card(s6, cur_x, y_pos, box_w, 2.0, fill_color=WHITE, border_color=p_col)
        
        box = s6.shapes.add_textbox(Inches(cur_x + 0.05), Inches(y_pos + 0.15), Inches(box_w - 0.1), Inches(1.7))
        tf_box = box.text_frame
        tf_box.word_wrap = True
        
        p = tf_box.paragraphs[0]
        p.text = f"Stage {idx+1}"
        p.font.size = Pt(11)
        p.font.bold = True
        p.font.color.rgb = BLUE
        p.alignment = PP_ALIGN.CENTER
        
        p = tf_box.add_paragraph()
        p.text = p_title
        p.font.size = Pt(13)
        p.font.bold = True
        p.font.color.rgb = p_col
        p.alignment = PP_ALIGN.CENTER
        
        p = tf_box.add_paragraph()
        p.text = p_sub
        p.font.size = Pt(11)
        p.font.color.rgb = DARK_TEXT
        p.alignment = PP_ALIGN.CENTER

    # Lower Card: Architecture Explanation
    add_card(s6, 0.8, 3.85, 11.7, 3.1, fill_color=BLUE_BG)
    b_desc = s6.shapes.add_textbox(Inches(1.1), Inches(4.0), Inches(11.1), Inches(2.8))
    tf = b_desc.text_frame
    tf.word_wrap = True

    p = tf.paragraphs[0]
    p.text = "How the End-to-End Pipeline Works (Sequential Dataflow):"
    p.font.size = Pt(18)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(10)

    arch_pts = [
        "Frontend (Review 1): The Source Program is scanned by the Lexer into typed Tokens. The Parser validates token order against the LL(1) grammar and constructs an in-memory Abstract Syntax Tree (AST).",
        "Middle-End (Review 2): The Semantic Analyzer verifies that variables and traffic states are compatible, storing identifiers in the Symbol Table. The IR Generator emits Three-Address Code (TAC) with temporaries (t1, t2).",
        "Backend (Review 3): The Optimizer simplifies constant conditions and eliminates dead branches. Finally, the Execution Engine tests simulated vehicle telemetry against the rules and outputs the Final Result."
    ]
    for pt in arch_pts:
        p = tf.add_paragraph()
        p.text = "• " + pt
        p.font.size = Pt(15)
        p.font.color.rgb = DARK_TEXT
        p.space_after = Pt(8)

    # =========================================================================
    # SLIDE 7: CHAPTER 5 (CONT.) - FINAL SYSTEM DESIGN & MODULES
    # =========================================================================
    s7 = prs.slides.add_slide(blank_layout)
    add_header(s7, "Chapter 5", "Final System Design & Module Description (lab5.pdf Sec 12)")

    # 2-Column layout for 9 modules
    add_card(s7, 0.8, 1.55, 5.6, 5.4)
    b7_1 = s7.shapes.add_textbox(Inches(1.1), Inches(1.8), Inches(5.0), Inches(4.9))
    tf = b7_1.text_frame
    tf.word_wrap = True

    p = tf.paragraphs[0]
    p.text = "Modules 1 to 5 (Frontend & Analysis)"
    p.font.size = Pt(18)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(12)

    m_left = [
        ("1. Lexical Analyzer (lexer.c)", "Converts source characters into tokens; strips comments (#); tracks line/col."),
        ("2. Syntax Analyzer (parser.c)", "Implements LL(1) recursive descent parsing to validate grammatical structure."),
        ("3. AST (parser.h)", "Stores hierarchical rule structure in memory; supports ASCII tree printing."),
        ("4. Semantic Analyzer", "Validates operand types (e.g. speed must be integer, signal must be state)."),
        ("5. Symbol Table", "Stores identifier attributes: name, type, value, and scope.")
    ]
    for m_name, m_detail in m_left:
        p = tf.add_paragraph()
        p.text = "• " + m_name
        p.font.size = Pt(15)
        p.font.bold = True
        p.font.color.rgb = BLUE
        p2 = tf.add_paragraph()
        p2.text = "   " + m_detail
        p2.font.size = Pt(13.5)
        p2.font.color.rgb = DARK_TEXT
        p2.space_after = Pt(6)

    # Right: Modules 6 to 9
    add_card(s7, 6.8, 1.55, 5.7, 5.4)
    b7_2 = s7.shapes.add_textbox(Inches(7.1), Inches(1.8), Inches(5.1), Inches(4.9))
    tf = b7_2.text_frame
    tf.word_wrap = True

    p = tf.paragraphs[0]
    p.text = "Modules 6 to 9 (IR, Optimization & Execution)"
    p.font.size = Pt(18)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(12)

    m_right = [
        ("6. IR Generator", "Translates AST into Three-Address Code (TAC) and Quadruples using temporaries."),
        ("7. Optimizer", "Performs constant folding and eliminates unreachable or redundant instructions."),
        ("8. Execution Engine", "Evaluates generated instructions using simulated vehicle sensor values."),
        ("9. Error Handler", "Reports unified diagnostics across all compiler stages with line and column numbers.")
    ]
    for m_name, m_detail in m_right:
        p = tf.add_paragraph()
        p.text = "• " + m_name
        p.font.size = Pt(15)
        p.font.bold = True
        p.font.color.rgb = GREEN if "Execution" in m_name else NAVY
        p2 = tf.add_paragraph()
        p2.text = "   " + m_detail
        p2.font.size = Pt(13.5)
        p2.font.color.rgb = DARK_TEXT
        p2.space_after = Pt(6)

    # =========================================================================
    # SLIDE 8: CHAPTER 6 - METHODOLOGY & LANGUAGE SPECIFICATION
    # =========================================================================
    s8 = prs.slides.add_slide(blank_layout)
    add_header(s8, "Chapter 6", "Methodology & Language Specification")

    # Left: Lexicon
    add_card(s8, 0.8, 1.55, 5.6, 5.4)
    b8_1 = s8.shapes.add_textbox(Inches(1.1), Inches(1.8), Inches(5.0), Inches(4.9))
    tf = b8_1.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Language Lexicon (lab5.pdf Sec 11)"
    p.font.size = Pt(19)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(12)

    lexicon = [
        ("Keywords", "VEHICLE, IF, ELSE, AND, OR"),
        ("Traffic States", "RED, GREEN, YELLOW, CROSSING, ROAD"),
        ("Rule Actions", "ISSUE_FINE, FLAG_VIOLATION, ALLOW"),
        ("Operators", "Assign:  =   |   Relational:  >,  <,  >=,  <=,  ==,  !="),
        ("Delimiters", "Block braces:  {   }"),
        ("Identifiers", "Variable names:  speed, signal, position, car"),
        ("Literals", "Integer numbers:  60, 75, 100")
    ]
    for cat, val in lexicon:
        p = tf.add_paragraph()
        p.text = f"• {cat}: "
        p.font.size = Pt(15)
        p.font.bold = True
        p.font.color.rgb = BLUE
        p.text += val
        p.font.color.rgb = DARK_TEXT
        p.space_after = Pt(6)

    # Right: Formal LL(1) Grammar
    add_card(s8, 6.8, 1.55, 5.7, 5.4)
    b8_2 = s8.shapes.add_textbox(Inches(7.1), Inches(1.8), Inches(5.1), Inches(4.9))
    tf = b8_2.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Formal LL(1) Grammar (EBNF)"
    p.font.size = Pt(19)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(10)

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
        p.font.size = Pt(12)
        p.font.name = "Consolas"
        p.font.color.rgb = NAVY
        p.space_after = Pt(3)

    # =========================================================================
    # SLIDE 9: CHAPTER 7 - COMPILER DESIGN CONCEPTS USED
    # =========================================================================
    s9 = prs.slides.add_slide(blank_layout)
    add_header(s9, "Chapter 7", "Compiler Design Concepts Used")

    concepts = [
        ("Deterministic Lexical Scanning (DFA)",
         "The lexer converts raw characters into discrete tokens using finite-state logic. Tracks line numbers and column positions for exact error reporting.",
         BLUE),
        ("Predictive Recursive Descent Parsing",
         "The parser is implemented through mutually recursive C functions (parse_statement, parse_if, parse_condition). Non-terminals map directly to C functions.",
         NAVY),
        ("Deterministic LL(1) Grammatical Properties",
         "The grammar is deterministic with 1 lookahead token. Production alternatives have mutually disjoint FIRST sets, eliminating backtracking.",
         GREEN),
        ("Grammar-Enforced Operator Precedence",
         "Operator precedence is encoded directly into the grammar hierarchy (OR -> AND -> Relational), guaranteeing correct associativity without requiring parentheses.",
         DARK_TEXT)
    ]
    for idx, (title, desc, color) in enumerate(concepts):
        y = 1.55 + idx * 1.35
        add_card(s9, 0.8, y, 11.7, 1.2)
        box = s9.shapes.add_textbox(Inches(1.1), Inches(y + 0.15), Inches(11.1), Inches(0.95))
        tf = box.text_frame
        tf.word_wrap = True
        
        p = tf.paragraphs[0]
        p.text = title
        p.font.size = Pt(18)
        p.font.bold = True
        p.font.color.rgb = color
        p.space_after = Pt(4)
        
        p2 = tf.add_paragraph()
        p2.text = desc
        p2.font.size = Pt(15)
        p2.font.color.rgb = DARK_TEXT

    # =========================================================================
    # SLIDE 10: CHAPTER 8 - IMPLEMENTATION DETAILS
    # =========================================================================
    s10 = prs.slides.add_slide(blank_layout)
    add_header(s10, "Chapter 8", "Implementation Details (Pure C99)")

    # Left: Source structure
    add_card(s10, 0.8, 1.55, 5.6, 5.4)
    b10_1 = s10.shapes.add_textbox(Inches(1.1), Inches(1.8), Inches(5.0), Inches(4.9))
    tf = b10_1.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Modular Source Organization"
    p.font.size = Pt(19)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(12)

    c_files = [
        ("src/lexer.h & src/lexer.c",
         "Defines TokenType enums and Token structs. Implements token scanning, keyword lookup, line/column tracking, and lexical error handling."),
        ("src/parser.h & src/parser.c",
         "Defines ASTNode structs (Program, Decl, Assign, If, Condition, Action). Implements recursive descent parser and hierarchical tree visualizer."),
        ("src/main.c",
         "CLI driver. Parses flags (--tokens, --ast, --all), reads source files, and orchestrates the compiler pipeline.")
    ]
    for fn, desc in c_files:
        p = tf.add_paragraph()
        p.text = fn
        p.font.size = Pt(15.5)
        p.font.bold = True
        p.font.color.rgb = BLUE
        p2 = tf.add_paragraph()
        p2.text = "  " + desc
        p2.font.size = Pt(14)
        p2.font.color.rgb = DARK_TEXT
        p2.space_after = Pt(6)

    # Right: Technical Highlights
    add_card(s10, 6.8, 1.55, 5.7, 5.4)
    b10_2 = s10.shapes.add_textbox(Inches(7.1), Inches(1.8), Inches(5.1), Inches(4.9))
    tf = b10_2.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Engineering Highlights"
    p.font.size = Pt(19)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(12)

    eng_pts = [
        "Implementation Language: Pure C (C99 standard).",
        "Compiler Toolchain: MinGW GCC 6.3.0.",
        "Zero External Dependencies: Written 100% from scratch — no Flex, Lex, Bison, or Yacc required.",
        "Zero Warning Build: Clean build under -Wall -Wextra with 0 warnings and 0 errors.",
        "AST Memory Management: Recursive ast_free() routine prevents memory leaks.",
        "One-Click Automation: build.bat compiles directly into trafficrule.exe."
    ]
    for pt in eng_pts:
        p = tf.add_paragraph()
        p.text = "• " + pt
        p.font.size = Pt(15)
        p.font.color.rgb = DARK_TEXT
        p.space_after = Pt(8)

    # =========================================================================
    # SLIDE 11: CHAPTER 9 - TESTING & RESULTS
    # =========================================================================
    s11 = prs.slides.add_slide(blank_layout)
    add_header(s11, "Chapter 9", "Testing & Prototype Results (Review 1)")

    # Left: Test Matrix
    add_card(s11, 0.8, 1.55, 5.6, 5.4)
    b11_1 = s11.shapes.add_textbox(Inches(1.1), Inches(1.8), Inches(5.0), Inches(4.9))
    tf = b11_1.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Test Matrix Verification (4/4 Passed)"
    p.font.size = Pt(19)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(12)

    tests = [
        ("Test 1: sample1_speed.tr", "Speed limit condition verified. Token stream and AST generated accurately.", GREEN),
        ("Test 2: sample2_compound.tr", "Compound AND expression verified. Nested binary AST generated.", GREEN),
        ("Test 3: Lexical Error Catching", "Input '@' symbol properly caught with line 3, col 12 error diagnostics.", GREEN),
        ("Test 4: Syntax Error Catching", "Malformed 'IF speed > {' caught with line 5 missing operand diagnostics.", GREEN)
    ]
    for name, desc, col in tests:
        p = tf.add_paragraph()
        p.text = "✔ " + name
        p.font.size = Pt(15.5)
        p.font.bold = True
        p.font.color.rgb = col
        p2 = tf.add_paragraph()
        p2.text = "    " + desc
        p2.font.size = Pt(13.5)
        p2.font.color.rgb = DARK_TEXT
        p2.space_after = Pt(6)

    # Right: AST Prototype output
    add_card(s11, 6.8, 1.55, 5.7, 5.4)
    b11_2 = s11.shapes.add_textbox(Inches(7.1), Inches(1.8), Inches(5.1), Inches(4.9))
    tf = b11_2.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Generated AST Output (Sample 1)"
    p.font.size = Pt(19)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(10)

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
    p.font.size = Pt(12.5)
    p.font.name = "Consolas"
    p.font.color.rgb = NAVY

    # =========================================================================
    # SLIDE 12: CHAPTER 10 - CONCLUSION
    # =========================================================================
    s12 = prs.slides.add_slide(blank_layout)
    add_header(s12, "Chapter 10", "Conclusion")

    add_card(s12, 0.8, 1.55, 11.7, 5.4)
    b12 = s12.shapes.add_textbox(Inches(1.1), Inches(1.8), Inches(11.1), Inches(4.9))
    tf = b12.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Review 1 Accomplishments & Summary"
    p.font.size = Pt(20)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(16)

    concl_pts = [
        "Language Grammar Formulated: Formalized a domain-specific LL(1) grammar and complete lexicon tailored specifically for traffic rule enforcement.",
        "Functional Lexer & Parser: Built a working scanner and predictive recursive descent parser from scratch in pure C99.",
        "In-Memory AST Generation: Constructed a robust hierarchical Abstract Syntax Tree and provided an ASCII visual tree formatter.",
        "Informative Error Diagnostics: Implemented precise lexical and syntax error messages with line numbers, column offsets, and pointers.",
        "Review 1 Prototype Complete: Verified through automated test suites (test_all.ps1), meeting 100% of the Review 1 requirements."
    ]
    for cp in concl_pts:
        p = tf.add_paragraph()
        p.text = "• " + cp
        p.font.size = Pt(16)
        p.font.color.rgb = DARK_TEXT
        p.space_after = Pt(12)

    # =========================================================================
    # SLIDE 13: CHAPTER 11 - FUTURE ENHANCEMENTS
    # =========================================================================
    s13 = prs.slides.add_slide(blank_layout)
    add_header(s13, "Chapter 11", "Future Enhancements (Project Roadmap)")

    f_cards = [
        ("Phase 2: Semantic Analysis", BLUE, [
            "Type checking (e.g. speed == RED disallowed).",
            "Symbol table to track scopes and identifiers.",
            "Undeclared identifier detection.",
            "Type incompatibility diagnostics."
        ]),
        ("Phase 2: IR & Optimizer", NAVY, [
            "Three-Address Code (TAC) generation.",
            "Quadruples representation table.",
            "Compile-time Constant Folding.",
            "Dead branch and code elimination."
        ]),
        ("Phase 3: Final Submission", GREEN, [
            "Lightweight interpreter virtual machine.",
            "Simulated vehicle telemetry evaluation.",
            "JSON/CSV batch violation test inputs.",
            "Formal documentation & final report."
        ])
    ]
    for idx, (title, col, items) in enumerate(f_cards):
        x = 0.8 + idx * (col_w + gap)
        add_card(s13, x, 1.55, col_w, 5.4)
        box = s13.shapes.add_textbox(Inches(x + 0.2), Inches(1.8), Inches(col_w - 0.4), Inches(4.9))
        tf = box.text_frame
        tf.word_wrap = True
        
        p = tf.paragraphs[0]
        p.text = title
        p.font.size = Pt(18)
        p.font.bold = True
        p.font.color.rgb = col
        p.space_after = Pt(14)
        
        for it in items:
            p = tf.add_paragraph()
            p.text = "• " + it
            p.font.size = Pt(15)
            p.font.color.rgb = DARK_TEXT
            p.space_after = Pt(10)

    # =========================================================================
    # SLIDE 14: REFERENCES & APPENDIX
    # =========================================================================
    s14 = prs.slides.add_slide(blank_layout)
    add_header(s14, "Sections 20 & 21", "References & Appendix")

    # Left: References
    add_card(s14, 0.8, 1.55, 5.6, 5.4)
    b14_1 = s14.shapes.add_textbox(Inches(1.1), Inches(1.8), Inches(5.0), Inches(4.9))
    tf = b14_1.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Academic References"
    p.font.size = Pt(19)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(12)

    references = [
        "1. Alfred V. Aho, Monica S. Lam, Ravi Sethi, and Jeffrey D. Ullman — 'Compilers: Principles, Techniques, and Tools' (2nd Ed., Pearson).",
        "2. Kenneth C. Louden — 'Compiler Construction: Principles and Practice' (Course Technology).",
        "3. Andrew W. Appel — 'Modern Compiler Implementation in C' (Cambridge University Press).",
        "4. BCSE307L Compiler Design Laboratory Course Manual & Project Guidelines, Fall 2026-27, VIT."
    ]
    for r in references:
        p = tf.add_paragraph()
        p.text = r
        p.font.size = Pt(14)
        p.font.color.rgb = DARK_TEXT
        p.space_after = Pt(8)

    # Right: Appendix & Q&A
    add_card(s14, 6.8, 1.55, 5.7, 5.4)
    b14_2 = s14.shapes.add_textbox(Inches(7.1), Inches(1.8), Inches(5.1), Inches(4.9))
    tf = b14_2.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = "Appendix: CLI Execution Commands"
    p.font.size = Pt(19)
    p.font.bold = True
    p.font.color.rgb = NAVY
    p.space_after = Pt(10)

    cmds = [
        ("Build Compiler", ".\\build.bat"),
        ("Print Tokens", ".\\trafficrule.exe --tokens examples\\sample1_speed.tr"),
        ("Print AST", ".\\trafficrule.exe --ast examples\\sample1_speed.tr"),
        ("Run All Stages", ".\\trafficrule.exe --all examples\\sample1_speed.tr"),
        ("Run Test Matrix", ".\\test_all.ps1")
    ]
    for label, cmd_str in cmds:
        p = tf.add_paragraph()
        p.text = f"• {label}:  {cmd_str}"
        p.font.size = Pt(13)
        p.font.bold = True
        p.font.color.rgb = BLUE
        p.space_after = Pt(4)

    p_qa = tf.add_paragraph()
    p_qa.text = "\nThank You! Questions & Discussion"
    p_qa.font.size = Pt(20)
    p_qa.font.bold = True
    p_qa.font.color.rgb = NAVY
    p_qa.alignment = PP_ALIGN.CENTER
    p_qa.space_after = Pt(4)

    p_sub = tf.add_paragraph()
    p_sub.text = "Alan James | 24BCT0181 | BCSE307L"
    p_sub.font.size = Pt(14)
    p_sub.font.color.rgb = DARK_TEXT
    p_sub.alignment = PP_ALIGN.CENTER

    # Save presentation
    output_file = "TrafficRule_Presentation.pptx"
    try:
        prs.save(output_file)
        print(f"Presentation successfully created: {output_file} ({len(prs.slides)} slides)")
    except PermissionError:
        output_file = "TrafficRule_Presentation_v2.pptx"
        prs.save(output_file)
        print(f"Note: TrafficRule_Presentation.pptx is currently open in PowerPoint (locked).")
        print(f"Presentation successfully created as: {output_file} ({len(prs.slides)} slides)")

if __name__ == "__main__":
    build_presentation()
