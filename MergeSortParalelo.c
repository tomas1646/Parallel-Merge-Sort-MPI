#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

void mergeSort(int *lista,int nroElementos);
void merge(int *izq,int nIzq,int *der,int nDer,int *lista);
void ImprimirLista(int lista[], int nroElementos);
int* mezclarArray(int *izq,int nIzq,int *der,int nDer);
void actualizarLista(int *lista, int *prueba, int elementos);

int main (int argc, char **argv)
{
	srand (time(NULL));
	
	int my_id,nproc;
	MPI_Status status;
	
	int NROELEMENTOS = atoi(argv[1]);

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_id);
	MPI_Comm_size(MPI_COMM_WORLD,&nproc);
	
	/*Asigno memoria dinamica, para las listas con muchos elementos*/
	int* lista;
	int size = NROELEMENTOS;
	lista = malloc(NROELEMENTOS*sizeof (int));
	
	int elementosAEnviar = NROELEMENTOS/nproc ;
	
	if(my_id == (nproc-1)){
		elementosAEnviar += NROELEMENTOS%nproc;
	}
	
	int* lista_recibo = malloc(elementosAEnviar*sizeof(int));
	
	if(my_id ==0){
		/*Master*/
		double start_t, end_t, total_t;
		
		/*Genera lista con numero aleatorios*/
		int i;
		for(i = 0; i<NROELEMENTOS; i++){
			lista[i] = rand() % 100;
		}
		
		printf("Arranco Contador Tiempo\n");
		start_t = MPI_Wtime();
		
		//printf("Lista ");
		//ImprimirLista(lista, NROELEMENTOS);
		int j;
		for(j=0; j<elementosAEnviar; j++){
			lista_recibo[j] = lista[j];
		}
		
		for(i = 1; i<nproc; i++){
			
			if(i == (nproc-1)){
				elementosAEnviar += NROELEMENTOS%nproc;
			}
			
			int* listaMandar;
			size = elementosAEnviar;
			listaMandar = malloc(size*sizeof(int));
			
			
			int j;
			for(j = 0; j<elementosAEnviar; j++){
				if(i == (nproc-1)){
					listaMandar[j] = lista[(elementosAEnviar-NROELEMENTOS%nproc)* i +j];
				}else{
					listaMandar[j] = lista[elementosAEnviar* i +j];
				}
			}
			
			//printf("Lista para %d ->",i);
			//ImprimirLista(listaMandar, elementosAEnviar);
			
			MPI_Send(listaMandar, elementosAEnviar, MPI_INT, i, 0, MPI_COMM_WORLD);
			
			free(listaMandar);
			
		}
		//Esto es para 'reiniciarle' al master, sino el elementosAEnviar, no coincide con su tamaño de lista_recibido
		elementosAEnviar -= NROELEMENTOS%nproc;
		
		//sleep(my_id);
		//printf("NODO %d ->",my_id);
		//ImprimirLista(lista_recibo, elementosAEnviar);
		
		mergeSort(lista_recibo, elementosAEnviar);
		
		for(i=0; i<elementosAEnviar; i++){
			lista[i] = lista_recibo[i];
		}
		
		free(lista_recibo);
		
		for(i = 1; i<nproc; i++){
			
			if(i == (nproc-1)){
				elementosAEnviar += NROELEMENTOS%nproc;
			}
			int lista_temp = malloc(elementosAEnviar*sizeof(int));
			
			MPI_Recv(lista_temp, elementosAEnviar, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
			
			//printf("Lista Temp i= %d ->",i);
			//ImprimirLista(lista_temp, elementosAEnviar);
			
			//printf("Lista   ANTES DE ORDENAR i= %d ->",i);
			//ImprimirLista(lista, NROELEMENTOS);
			
			if(i == (nproc-1)){
				elementosAEnviar -= NROELEMENTOS%nproc;
			}
			
			int *prueba;
			
			
			if(i == (nproc-1)){
				prueba = malloc(NROELEMENTOS*sizeof(int));
				
				prueba = mezclarArray(lista,i*elementosAEnviar,lista_temp,elementosAEnviar + NROELEMENTOS%nproc);
				actualizarLista(lista, prueba, NROELEMENTOS);
			}else{
				prueba = malloc(((i+1)*elementosAEnviar)*sizeof(int));
				
				prueba = mezclarArray(lista,i*elementosAEnviar,lista_temp,elementosAEnviar);
				actualizarLista(lista, prueba, (i+1)*elementosAEnviar);
			}
			free(prueba);
			free(lista_temp);
			//printf("Lista DESPUES DE ORDENAR i= %d ->",i);
			//ImprimirLista(lista, NROELEMENTOS);
			
		}
		
		end_t = MPI_Wtime();
		total_t = (end_t - start_t);
		printf("Termino Programa: segundos %f\n",total_t);
		
		free(lista);
		
		//printf("\nLista ordenada ->");
		//ImprimirLista(lista, NROELEMENTOS);
	}else{
		/*Worker*/
		
		MPI_Recv(lista_recibo, elementosAEnviar, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		
		//sleep(my_id);
		//printf("NODO %d ->",my_id);
		//ImprimirLista(lista_recibo, elementosAEnviar);
		
		mergeSort(lista_recibo, elementosAEnviar);
		
		
		//sleep(my_id);
		//printf("NODO %d ->",my_id);
		//ImprimirLista(lista_recibo, elementosAEnviar);
		
		MPI_Send(lista_recibo, elementosAEnviar, MPI_INT, 0, 0, MPI_COMM_WORLD);
		
		free(lista);
		free(lista_recibo);
	}
	
	MPI_Finalize();	
}

void mergeSort(int *lista,int nroElementos)
{
	if(nroElementos==1){return;}
	
	int mitad = nroElementos / 2;
	
	int* izq = malloc(mitad*sizeof(int));
	int* der = malloc((nroElementos-mitad)*sizeof(int));
	
	int i;
	for(i=0;i<mitad;i++){
		izq[i] = lista[i];
	}
	
	for(i=mitad;i<nroElementos;i++){
		der[i-mitad] = lista[i];
	}
	
	mergeSort(izq, mitad);
	mergeSort(der, nroElementos-mitad);
	merge(izq, mitad, der, nroElementos-mitad, lista);
	
	free(izq);
	free(der);
}

void actualizarLista(int *lista, int *prueba, int elementos){
	int i;
	for(i = 0; i<elementos; i++){
		lista[i] = prueba[i];
	}
}

int* mezclarArray(int *izq,int nIzq,int *der,int nDer)
{
	int* lista_definitiva;
	int size = nIzq + nDer;
	
	lista_definitiva = malloc(size*sizeof(int));
	
	int i=0,j=0,k=0;
	/*Compara array Izquierdo y Derecho y va ordenando en la lista de menor a mayor*/
	while( ( i < nIzq ) && ( j < nDer ) )
	{
		if( izq[i] <= der[j])
		{
			lista_definitiva[k] = izq[i];
			i++;
		}
		else
		{
			lista_definitiva[k] = der[j];
			j++;
		}
		k++;
	}
	/*El array derecho ya se recorrio entero*/
	while(i < nIzq)
	{
		lista_definitiva[k] = izq[i];
		i++;k++;
	}
	/*El array izquierdo ya se recorrio entero*/
	while(j < nDer)
	{
		lista_definitiva[k] = der[j];
		j++;k++;
	}
	return lista_definitiva;
}

void merge(int *izq,int nIzq,int *der,int nDer,int *lista)
{
	int i=0,j=0,k=0;
	/*Compara array Izquierdo y Derecho y va ordenando en la lista de menor a mayor*/
	while( ( i < nIzq ) && ( j < nDer ) )
	{
		if( izq[i] <= der[j])
		{
			lista[k] = izq[i];
			i++;
		}
		else
		{
			lista[k] = der[j];
			j++;
		}
		k++;
	}
	/*El array derecho ya se recorrio entero*/
	while(i < nIzq)
	{
		lista[k] = izq[i];
		i++;k++;
	}
	/*El array izquierdo ya se recorrio entero*/
	while(j < nDer)
	{
		lista[k] = der[j];
		j++;k++;
	}
}

void ImprimirLista(int lista[], int nroElementos)
{
	int i;
	
	for (i = 0; i < nroElementos; i++)
		printf("%d ", lista[i]);
	printf("\n");
}
