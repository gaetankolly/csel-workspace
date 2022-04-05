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
  char text[ELEMENT_STRING_SIZE]; //param
  int element_id;                 // param
  struct list_head node;          // element liste_head which allow to move in the chained list
};

static LIST_HEAD(list); // head of the list, global variable

// allocate on element and add it at the tail of the list
void alloc_element (int id) {
	struct element* newElement;
	newElement = kzalloc(sizeof(*newElement), GFP_KERNEL); // create a new element on heap, GFP_KERNEL= allocation standard
	if (newElement != NULL){
		newElement->element_id=id;
		strncpy(newElement->text, elementText, ELEMENT_STRING_SIZE-1);
		list_add_tail(&newElement->node, &list); // add element at the end of the list
	}
}

// printlist
void printList(void) {
  struct element* ele;
  list_for_each_entry(ele, &list, node) {
      pr_info("Text= %s, ID= %d\n",ele->text, ele->element_id);
  }
}

// deallocate element of the list
void removeCompleteList(void) {
  struct element* ele;
	while (!list_empty (&list)) {
    ele=list_last_entry(&list, struct element, node);
    pr_debug("Delete: %d\n", ele->element_id);
		list_del (&ele->node);
		kfree (ele);
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
  pr_debug("INIT:    text: %s\t  elements: %d\n", elementText, nbElements);
  // display list
  printList ();
	// dealloc list
  removeCompleteList();
	pr_info ("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Andrea Enrile; Gaëtan Kolly");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");