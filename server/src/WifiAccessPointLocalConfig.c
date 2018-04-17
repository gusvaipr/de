#include <WifiAccessPointLocalConfig.h>

static xmlNode * findNode(const char *path)
{
   xmlNode *curr = firstRootChild;

   char *pathName = strstr(path,"/WifiNodes/");

   if(pathName == NULL)
   {
      return NULL;
   }

   pathName += strlen("/WifiNodes/");

   while((curr != NULL)  && (pathName != NULL))
   {
      curr = xmlNextElementSibling(curr);
      if (curr)
      {
         size_t len = strlen((const char *)curr->name);
         if(!strncmp(pathName,(const char*)curr->name,len))
         {

            curr = curr->children;
            pathName = strstr((const char*)pathName,"/");
            if (pathName)
            {
               //printf("match: %s\n",pathName);
               pathName++;
            }
         }
      }
   }
   return curr;
}

int32_t lcfg_getFloatParameter(const char *path,float *value)
{
   xmlNode *myNode = findNode(path);

   if (myNode != NULL)
   {
      pthread_mutex_lock(&lock);
      *value = strtof((char*)myNode->content,NULL);
      pthread_mutex_unlock(&lock);
      return 0;
   }
   else
   {
      *value = 0.0f;
      return -1;
   }
}

int32_t lcfg_getInt32Parameter(const char *path, int32_t *value)
{
   xmlNode *myNode = findNode(path);

   if (myNode != NULL)
   {
      pthread_mutex_lock(&lock);
      *value = (int32_t)strtol((char*)myNode->content,NULL,0);
      pthread_mutex_unlock(&lock);
      return 0;
   }
   else
   {

	   *value = 0;
	   return -1;
   }
}

char * lcfg_getStringParameter(const char *path)
{
   xmlNode *myNode = findNode(path);
   xmlChar * ret = NULL;

   if (myNode != NULL)
   {
      pthread_mutex_lock(&lock);
      ret = xmlNodeGetContent(myNode);
      pthread_mutex_unlock(&lock);
      return (char *)ret;
   }
   else
   {
      return NULL;
   }
}

int32_t lcfg_setStringParameter(const char *path, const char *content)
{
   xmlNode *myNode;

   if (path && content)
   {
      myNode = findNode(path);

      if (myNode)
      {
         xmlNodeSetContent(myNode,(const xmlChar *)content);
         return 0;
      }
      else
      {
         return -1;
      }
   }
   else
   {
      return -1;
   }
}

void lcfg_freeStringParameter(char *parameter)
{
   if (parameter != NULL)
   {
      xmlFree(parameter);
   }
}

#define PRELOAD_TO_MEMORY
int32_t lcfg_initialize(const char* lcfg_file)
{
   struct timespec before,after;
   long delta;
   static xmlNode *root = NULL;
   static xmlDoc * doc = NULL;
   int32_t  validationResult = (-1);

#ifdef PRELOAD_TO_MEMORY
   struct stat fstatstruct;
   int fin,ret;
   size_t bytesRead=0;
   char *inputXml;
#endif

   clock_gettime(CLOCK_MONOTONIC_RAW, &before);
   pthread_mutex_init(&lock,NULL);

#ifdef PRELOAD_TO_MEMORY
   fin = open(lcfg_file,O_RDONLY);

   if (fin != -1)
   {

      ret = fstat(fin,&fstatstruct);

      if (ret == 0)
      {
         inputXml = (char*)malloc((uint32_t)fstatstruct.st_size);

         if (inputXml == NULL)
         {
        	 printf("[%s] - malloc failed !!!! \n", __func__);

            close(fin);
            return -1;
         }
      }
      else
      {
    	  printf("[%s] - fstat in lcfg failed \n", __func__);

         close(fin);
         return -1;
      }

      while(bytesRead < (size_t)fstatstruct.st_size)
      {
         bytesRead += (size_t)read(fin,&inputXml[bytesRead],(size_t)fstatstruct.st_size - bytesRead);
      }

      doc = xmlReadMemory(inputXml,(int32_t)fstatstruct.st_size,"noname.xml", NULL, 0);

      free(inputXml);
      close(fin);
   }

#else
   doc = xmlReadFile(lcfg_file,NULL,0);
#endif

   if (doc != NULL)
   {
	   printf("[%s] - %s loaded OK !!! \n", __func__, lcfg_file);

      validationResult = 0; //ValidateXml(doc, lcfg_file);
   }

   if ((NULL == doc) || (0 != validationResult))
   {
	   printf("[%s] - %s is NOK, No LCFG used!! \n", __func__, lcfg_file);

   }

   clock_gettime(CLOCK_MONOTONIC_RAW, &after);
   delta = 1000000000*(after.tv_sec - before.tv_sec) + after.tv_nsec - before.tv_nsec;
   printf("[%s] - loading and indexing took %ld ns\n", __func__, delta);

   root = xmlDocGetRootElement(doc);

   if (root == NULL)
   {
      return -1;
   }
   else if ((strcmp((const char *)root->name,"WifiNodes") != 0))
   {
	   printf("[%s] - root element \"/WifiNodes\" must be present, found only %s\n", __func__, root->name);

      return -1;
   }
   else
   {
      firstRootChild = root->children;
      if (firstRootChild == NULL)
      {
    	  printf("[%s] - Could not find any children to root element \"/wifiNodes\"\n", __func__);
         return -1;
      }
   }

   return 0;
}