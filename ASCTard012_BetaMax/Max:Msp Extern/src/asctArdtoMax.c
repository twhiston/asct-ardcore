/**
 @file
 asctArdtoMax - ardcore to max/msp object.
 Deals with serial message decode and send for the Ardcore Eurorack module
 Unlike messages from the ardcore messages sent to it have no transaction start message
 
 Thomas Whiston
 tom.whiston@gmail.com
 
 @ingroup	asctSoftware
 */

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object

//Some character codes we use as id's when sending messages to the ardcore
#define XOFF 19
#define DIGIVAL 100
#define ANVAL 97

#define INBUFFSIZE 32
////////////////////////// object struct
typedef struct _asctArdtoMax
{
	t_object ob;		// the object itself (must be first)
    long m_in;          //space for inlet number for proxy
    void *m_proxy;      //proxy input
    void *m_outlet1;    // outlet creation- we need lots!!
    void *m_outlet2;
    void *m_outlet3;
    void *m_outlet4;
    void *m_outlet5;
    void *m_outlet6;
    void *m_outlet7;
    void *m_outlet8;
    
    char serialIn[INBUFFSIZE]; //hold input data here for processing
    t_uint8 serialIndex; // the index of where we are writing in the serialIn array
    
} t_asctArdtoMax;

///////////////////////// function prototypes
//// standard set
void *asctArdtoMax_new(t_symbol *s, long argc, t_atom *argv);
void asctArdtoMax_free(t_asctArdtoMax *x);
void asctArdtoMax_assist(t_asctArdtoMax *x, void *b, long m, long a, char *s);
void asctArdtoMax_int(t_asctArdtoMax *x, long n);
void asctArdtoMax_list(t_asctArdtoMax *x, t_symbol *s, long argc, t_atom *argv);
//////////////////////// global class pointer variable
void *asctArdtoMax_class;

int C74_EXPORT main(void)
{
	// object initialization
	t_class *c;
	
	c = class_new("asctArdtoMax", (method)asctArdtoMax_new, (method)asctArdtoMax_free, (long)sizeof(t_asctArdtoMax),
				  0L /* leave NULL!! */, A_GIMME, 0);
	
    class_addmethod(c, (method)asctArdtoMax_assist,"assist",A_CANT, 0);
    class_addmethod(c, (method)asctArdtoMax_int,"int",A_LONG, 0);	// the method for an int in the left inlet (inlet 0)
    class_addmethod(c, (method)asctArdtoMax_list,"list",A_GIMME, 0);
	
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    
	asctArdtoMax_class = c;
    
	return 0;
}

void *asctArdtoMax_new(t_symbol *s, long argc, t_atom *argv)
{
	t_asctArdtoMax *x = NULL;
    
	if (x = (t_asctArdtoMax *)object_alloc(asctArdtoMax_class)) {
        x->m_proxy = proxy_new((t_object *)x, 2, &x->m_in);
        x->m_proxy = proxy_new((t_object *)x, 1, &x->m_in);
        x->m_outlet8 = bangout((t_object *)x);
        x->m_outlet7 = listout((t_object *)x);
        x->m_outlet6 = listout((t_object *)x);
        x->m_outlet5 = listout((t_object *)x);
        x->m_outlet4 = listout((t_object *)x);
        x->m_outlet3 = listout((t_object *)x);
        x->m_outlet2 = listout((t_object *)x);
        x->m_outlet1 = listout((t_object *)x);
	}
	return (x);
}

void asctArdtoMax_free(t_asctArdtoMax *x)
{
	freeobject((t_object *)x->m_proxy); //free our proxy
    //do we need to free our outputs???
}

/**
 * Proxy list input, only valid on input 3 for digi ins, the rest just return
 */
void asctArdtoMax_list(t_asctArdtoMax *x, t_symbol *s, long argc, t_atom *argv)
{
	t_atom *ap;
    t_uint8 i;
    t_uint8 in;
    char port[2];
    char val[2];

	if (proxy_getinlet((t_object *)x) == 0)
        return;
    if (proxy_getinlet((t_object *)x) == 1)
        return;
    
    
    /**
     * Inlet 2 deals with digital ons
     * message to serial is
     * [o][d][INDEX][VALUE]
     */
    if (proxy_getinlet((t_object *)x) == 2){
        //now we have to pass the index and value
        for(i= 0, ap = argv; i<argc;i++, ap++)
        {
            switch (atom_gettype(ap)) {
                case A_LONG:
                    in = atom_getlong(ap);
                    //these returns safeguard against incorrect values
                    if (i == 0 && in > 9) {
                        return;
                    }
                    if (i == 1 && in > 1) {
                        return;
                    }
                    //turn our longs into chars
                    if (i == 0) {
                        sprintf(port, "%d", in);
                    }
                    if (i == 1) {
                        sprintf(val, "%d", in);
                    }
                    break;
                case A_FLOAT:
                    break;
                case A_SYM:
                    break;
                default:
                    break;
            }
        }
        //write the outputs
        outlet_int(x->m_outlet1, DIGIVAL);//d
        outlet_int(x->m_outlet1, port[0]);
        outlet_int(x->m_outlet1, val[0]);
        outlet_int(x->m_outlet1, XOFF);
    }
}

void asctArdtoMax_int(t_asctArdtoMax *x,long n)
{
	t_int8 i = 0;
    if (proxy_getinlet((t_object *)x) == 0){
        //transmission start flag
        if (n == 17) {
            //when this flag is set we have a new serial message so we reset the position
            x->serialIndex = 0;
            return;
        }
        //transmission stop flag
        if(n != 19){
            //19 is the XOFF character
            x->serialIn[x->serialIndex] = n;
            //safeguard against writing outside of allocated space
            if(x->serialIndex < INBUFFSIZE-1){
                x->serialIndex++;
            } else {
               x->serialIndex = 0;
            }
            return;
        }
        if(n == 19){
            t_int16 temp = 0;
            //if the input is not indexed as 6 or 7 we know we must combine 2 bytes to get the output value
            if (x->serialIn[0] != 6 || x->serialIn[0] != 7)
                temp = (x->serialIn[2] & 127) + (x->serialIn[1] << 7);
            
            /**
             * now that we have the value we need to send it to the right output
             * cases 0-5 describe inputs that should be routed to the outputs
             * 6+ are special case messages, most of these go to m_outlet1 which is for serial transmission
             * 6 - respond to handshake message
             */
            switch (x->serialIn[0]) {
                case 0:
                    outlet_int(x->m_outlet2, temp);
                    break;
                case 1:
                    outlet_int(x->m_outlet3, temp);
                    break;
                case 2:
                    outlet_int(x->m_outlet4, temp);
                    break;
                case 3:
                    outlet_int(x->m_outlet5, temp);
                    break;
                case 4:
                    outlet_int(x->m_outlet6, temp);
                    break;
                case 5:
                    outlet_int(x->m_outlet7, temp);
                    break;
                case 6:
                    outlet_int(x->m_outlet1, XOFF);//case 6 is the handshake, we return a byte, value doesnt matter
                case 7:
                    outlet_bang(x->m_outlet8);//case 7 is a clk input so we do a bang
                default:
                    break;
            }
        }
        return;
    }
    
    
    if (proxy_getinlet((t_object *)x) == 1){
        /**
         * input 1 is for the DAC out, we need to take the input and prepare a message with it
         * output message is
         * [a][VALUE]
         */
        char txt[4];
        //safeguard against the value being out of bounds
        if(n > 255)
            n = 255;
        if(n<0)
            n = 0;
        sprintf(txt, "%ld", n); //int to char conversion
        //now send the message
        outlet_int(x->m_outlet1, ANVAL);//a
        for (i = 0; i<3; i++) {
            //strip out 0's apart from in the case of the first index where 0 is allowed
            if (txt[i] != 0) {
                outlet_int(x->m_outlet1, txt[i]);
            }
            if(txt[i] == 0 && i == 0)
                outlet_int(x->m_outlet1, txt[i]);
        }
        outlet_int(x->m_outlet1, XOFF);
        return;
    }
    
    if (proxy_getinlet((t_object *)x) == 2)
        return;
    
    
    return;
}

void asctArdtoMax_assist(t_asctArdtoMax *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
        if(a == 0)
            sprintf(s, "Serial In");
        else if(a == 1)
            sprintf(s, "To DAC");
        else if(a == 2)
            sprintf(s, "To DIGITAL OUT");
	}
	else {	// outlet
		if(a == 0)
            sprintf(s, "Serial Out");
        else if(a == 7)
            sprintf(s, "Clk Out Bang");
        else
            sprintf(s, "Ardcore Out A[%ld]", a);
	}
}