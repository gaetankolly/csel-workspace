// skeleton.c
#include <linux/module.h> // needed by all modules
#include <linux/init.h>   // needed for macros
#include <linux/kernel.h> // needed for debugging

#include <linux/moduleparam.h> // needed for module parameters

/* function needed for list */
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/string.h>

// parameters
static char *elementText = "This is a list of element"; // First element
module_param(elementText, charp, 0664);
static int nbElements = 10; // nb of element which will be created
module_param(nbElements, int, 0);

// list
#define ELEMENT_STRING_SIZE (100)
struct element
{
  char text[ELEMENT_STRING_SIZE];
  int element_id;
  struct list_head node;
};

static LIST_HEAD(list); // head of the list, global variable

// allocate on element and add it at the tail of the list
void alloc_element(int id)
{
  struct element *newElement;
  newElement = kzalloc(sizeof(*newElement), GFP_KERNEL); // create a new element on heap
  pr_debug("ALLOC:  element %d created\n", id);
  if (newElement != NULL)
  {
    newElement->element_id = id;
    strncpy(newElement->text, elementText, ELEMENT_STRING_SIZE - 1);
    list_add_tail(&newElement->node, &list); // add element at the end of the list
  }
}

static int __init skeleton_init(void)
{
  int i;
  pr_info("INIT:  Linux module skeleton loaded\n");
  pr_debug("INIT:    text: %s\t  elements: %d\n", elementText, nbElements);

  for (i = 0; i < nbElements; i++)
  {
    alloc_element(i);
  }

  return 0;
}

static void __exit skeleton_exit(void)
{
  // display list

  // dealloc list

  pr_info("EXIT:  Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Andrea Enrile; Gaëtan Kolly");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");