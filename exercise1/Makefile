CC=mpicc -fopenmp
loc ?= .
OBJDIR=obj

OBJECTS=$(OBJDIR)/main.o

par: $(loc)/main.x

$(loc)/main.x: $(OBJECTS)
	$(CC) -lm $(OBJECTS) -o $@

$(OBJDIR)/%.o: %.c pgm.h evolution.h dev.h
	@mkdir -p $(OBJDIR)
	$(CC) -c $< -o $@

clean: x_clean

x_clean:
	@rm -rf $(loc)/*.x $(OBJDIR)/*.o $(OBJDIR)/*.a