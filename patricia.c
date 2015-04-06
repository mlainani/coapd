patricia insert(key, t)
typekey key;
patricia t;
{
    patricia p;
    patricia InsBetween();
    int i;
    if (t == NULL)
	return (NewDataNode(key));
    for (p = t; !IsData(p);)
	p = bit(p->level, key) ? p->right : p->left;	/* find first different bit */
    for (i = 1; i <= D && bit(i, key) == bit(i, p->k); i++);
    if (i > D) {
	Error /* Key already in table */ ;
	return (t);
    } else
	return (InsBetween(key, t, i));
}

patricia InsBetween(key, t, i)
typekey key;
patricia t;
int i;
{
    patricia p;
    if (IsData(t) || i < t->level) {	/* create a new internal node */
	p = NewDataNode(key);
	return (bit(i, key) ? NewIntNode(i, t, p) : NewIntNode(i, p, t));
    }
    if (bit(t->level, key) == 1)
	t->right = InsBetween(key, t->right, i);
    else
	t->left = InsBetween(key, t->left, i);
    return (t);
};
