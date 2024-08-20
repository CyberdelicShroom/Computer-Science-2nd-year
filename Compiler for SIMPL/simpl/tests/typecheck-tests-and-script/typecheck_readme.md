# Setup:

---

- Merge **input/** and **output/** with your existing directories in **simpl/tests/**  
- Copy **otherscript.py** to **simpl/tests/**  
- Run as usual, with the option **typecheck** to run that suite.  

---

# Valgrind:

---

Your code will pass this script's memory check (**memcheck**) if either:  

*(a)* The **valgrind** output says that **no leaks are possible**.   
**OR**  
*(b)* The **SIMPL** compiler terminates with an error (e.g: 'not an array')  
and the **valgrind** output states that some memory is 'still reachable'.  
   
Anything else will be treated as *unknown valgrind output*. The **valgrind**  
summary will be printed as well as the input file and your **SIMPL**  
compiler's output.  

---

# Notes:

---

- Set the variable **askToSave** to **False** if you don't want to be   
  prompted to save your output to an 'expected output' file.   
  (Your output will not be saved)  
- Set the variable **memcheck** to **True** if you want to run **valgrind**   
  on all the test cases. Set it to false if you don't want to run **valgrind**.
- If you have **memcheck** set to **True**, set **requestDetails** to **True**  
  if you want to be prompted for **valgrind's** full output.  
- I'm fairly confident that the 'expected output' files are correct, but not  
  100% sure.  

---

# Acknowledgement:

---

**otherscript.py** is an updated version of Aaron's **testscript.py** and all
the extra code is inspired by, and is based on, his original code.

---
