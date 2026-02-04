You are a senior UI/UX engineer, creative director, and frontend architect.

You are working on an existing Flask-based web application called:
“Adaptive Library Search, Issue & Recommendation System”.

IMPORTANT CONTEXT
- A premium dark-mode UI already exists.
- The design uses glassmorphism, gradients, cards, smooth animations.
- HTML templates already exist: login.html, home.html, search.html.
- Backend logic is in C++ and Flask APIs already exist.
- You MUST preserve the current design language, layout structure, fonts, colors, and animation style.
- Do NOT redesign from scratch.
- Do NOT remove any existing functionality.
- Enhance, refine, and extend instead.

====================================================
PRIMARY GOAL
====================================================
Improve the UI to feel even more premium, dynamic, and interactive,
while adding new meaningful pages and components that make the system
feel like a real product—not a student project.

====================================================
NEW FEATURE: USER PROFILE PAGE (CRITICAL)
====================================================
Create a new page:
- profile.html

This page should visually match the existing UI style.

The profile page must display:
1. User Information Card
   - User name
   - User ID
   - User type (Student / Final-Year / Faculty)
   - Registration date
   - Styled badge based on user type

2. Borrowed Books Section
   - Card/list view of currently borrowed books
   - Book title, author
   - Issue date
   - Return-by date
   - Status badge (On Time / Due Soon / Overdue)
   - Action button: “Return Book”

3. Activity Timeline
   - Recent actions:
     - Issued book
     - Returned book
     - Reserved book
   - Vertical timeline with subtle animations

4. Statistics Widgets
   - Total books borrowed
   - Active borrows
   - Reservations
   - Recommended books count

NOTE:
- Data can be assumed to come from backend APIs later.
- For now, structure UI components cleanly and logically.
- Use placeholders only for values, NOT for logic.

====================================================
UI / UX ENHANCEMENTS (MANDATORY)
====================================================
Enhance the existing pages with:

1. Navigation Improvements
   - Add “Profile” link in navbar
   - Active page highlighting
   - Smooth hover & magnetic effects

2. Micro-Interactions
   - Button hover depth
   - Card hover elevation
   - Smooth fade & slide-in animations
   - Loading shimmer placeholders

3. Visual Polish
   - Refine spacing, typography hierarchy
   - Improve glassmorphism depth
   - Use subtle glow & gradient accents
   - Ensure responsive behavior

4. Search Page Enhancements
   - Animated result entry
   - Smooth grid/list toggle transitions
   - Clear empty and loading states
   - Keyboard focus styles

====================================================
TECHNICAL CONSTRAINTS (NON-NEGOTIABLE)
====================================================
- Use ONLY HTML, CSS, and minimal JavaScript for animations.
- DO NOT perform any business logic in JavaScript.
- DO NOT change Flask routes or backend APIs.
- DO NOT introduce frameworks (React, Vue, etc.).
- Keep everything compatible with Flask templates.
- Reuse existing CSS classes where possible.
- Add new CSS only when necessary.

====================================================
DELIVERABLES
====================================================
1. Updated versions of:
   - home.html
   - search.html
2. New file:
   - profile.html
3. Any necessary CSS additions (append to existing style.css)
4. Any small JS animation enhancements (append to animations.js)

====================================================
QUALITY BAR
====================================================
This UI must feel:
- Premium
- Cinematic
- Recruiter-impressive
- Startup-grade
- Clean and academically presentable

It should look like something that could win a UI/UX award,
while still being explainable in a viva.

Proceed carefully, preserve existing structure,
and enhance—not replace—what already exists.
